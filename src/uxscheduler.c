/*
 * uxscheduler.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include <stdint.h>

#define AWE_LOG_TAG "uxscheduler"
#include "awe/log.h"

#include "awe/awe.h"

#include <ux.h>
#include "awe/uxscheduler.h"

#define UXFD_SIZE 256

typedef struct uxevent{
	struct list_head _head; // list item

	UDTSOCKET _ufd;
	int _conditionSet;

	sockethandler_proc* _proc;//uxevent_proc
	void* _userdata;
}uxevent;

static uxevent* uxevent_create(UDTSOCKET ufd, int conditionSet,
		sockethandler_proc* proc, void* userdata){
	uxevent *uxe = awe_mallocz(sizeof(uxevent));
	ALOGV("create, uxevent(%p).", uxe);
	uxe->_ufd = ufd;
	uxe->_conditionSet = conditionSet;
	uxe->_proc = proc;
	uxe->_userdata = userdata;
	return uxe;
}
static void uxevent_destroy(uxevent* uxe){
	awe_free(uxe);
}

//--------------------------------------------------------------

struct awe_uxscheduler{
	volatile awe_bool _started;

	awe_mutex_t		*_mutex;
	awe_cond_t		*_cond;
	awe_thread_t		*_thread;

	struct list_head _uxevent_list;

	timer_task_list _task_list;

	int32_t _delayMs;

	int32_t _have_rmopt;
};

static int uxevent_list_size(struct list_head *lhead){
	int count = 0;
	struct list_head *pos = NULL;
	list_for_each(pos, lhead)
	{
		++count;
	}
	return count;
}

static uxevent* uxscheduler_uxevent_find(struct list_head *lhead, UDTSOCKET ufd){
	uxevent *entry = NULL;
	struct list_head *pos = NULL;
	list_for_each(pos, lhead)
	{
		entry = list_entry(pos, uxevent, _head);
		if(entry->_ufd == ufd){
			return entry;
		}
	}
	return NULL;
}

static int uxscheduler_thread_process(awe_thread_t* t, void* userdata){
	awe_uxscheduler_t* uxsched = (awe_uxscheduler_t*)userdata;
	while(!awe_thread_exit_pending(t)){
		awe_mutex_lock(uxsched->_mutex);
		while(uxsched->_started && list_empty(&uxsched->_uxevent_list)){
			awe_cond_wait(uxsched->_cond, uxsched->_mutex);
		}
		awe_mutex_unlock(uxsched->_mutex);
		if(!uxsched->_started){
			break;
		}

		awe_mutex_lock(uxsched->_mutex);
		int ufd_size = uxevent_list_size(&uxsched->_uxevent_list);

	    UFD_SET_DECLARE(rset, ufd_size);
	    UFD_ZERO(&rset);
	    UFD_SET_DECLARE(wset, ufd_size);
	    UFD_ZERO(&wset);

//	    UFD_SET_DECLARE(eset, ufd_size);
//	    UFD_ZERO(&eset);

		uxevent *uxe = NULL;
		struct list_head *pos = NULL;
		list_for_each(pos, &uxsched->_uxevent_list)
		{
			uxe = list_entry(pos, uxevent, _head);
//			AWE_CHECK_NE(0, uxe->_ufd);

		    if (uxe->_conditionSet&AWE_READ) UFD_SET(uxe->_ufd, &rset);
		    if (uxe->_conditionSet&AWE_WRITE) UFD_SET(uxe->_ufd, &wset);
//		    UFD_SET(uxe->_ufd, &eset);
		}

		int32_t delayms = uxsched->_delayMs;

		if(!timer_task_list_empty(&uxsched->_task_list)){
			int64_t now_time = awe_time_ms();
			timer_task* task = timer_task_list_first(&uxsched->_task_list);
			int32_t timeToDelay = (int32_t)(task->_proc_ms - now_time);
			if(timeToDelay >= 0 && timeToDelay < delayms){
				delayms = timeToDelay;
			}else if(timeToDelay < 0){
				delayms = 0;
			}
		}

		awe_mutex_unlock(uxsched->_mutex);

	    struct timeval tv;
	    struct timeval *ptv = NULL;

	    if(delayms >= 0) {
	        // Calculate timeout value
	        tv.tv_sec = delayms / 1000;
	        tv.tv_usec = (delayms - (tv.tv_sec * 1000)) * 1000;
	        ptv = &tv;
	    }

	    int selectResult = 0;
	    if(ufd_size > 0){
	    	selectResult = ux_select(ufd_size, &rset, &wset, NULL/*&eset*/, ptv);
		}
	    if(selectResult > 0){
	    	int i=0;
	    	for(i=0; i<rset.length; i++){
	    		if(rset.ufd_arr[i] != 0){
	    			uxevent* uxe = uxscheduler_uxevent_find(&uxsched->_uxevent_list, rset.ufd_arr[i]);
	    			if(uxe != NULL){
	    				uxe->_proc(uxe->_userdata, rset.ufd_arr[i], AWE_READ);
	    			}
	    		}
	    	}
	    	for(i=0; i<wset.length; i++){
	    		if(wset.ufd_arr[i] != 0){
	    			uxevent* uxe = uxscheduler_uxevent_find(&uxsched->_uxevent_list, wset.ufd_arr[i]);
	    			if(uxe != NULL){
	    				uxe->_proc(uxe->_userdata, wset.ufd_arr[i], AWE_WRITE);
	    			}
	    		}
	    	}
//	    	for(i=0; i<eset.length; i++){
//	    		if(eset.ufd_arr[i] != 0){
//	    			uxevent* uxe = uxscheduler_uxevent_find(&uxsched->_uxevent_list, eset.ufd_arr[i]);
//	    			if(uxe != NULL){
//	    				uxe->_proc(uxe->_userdata, eset.ufd_arr[i], UX_EXCEPTION);
//	    			}
//	    		}
//	    	}

	    }else if(selectResult < 0){
	    	ALOG_ASSERT(selectResult < 0);
	    }

	    timer_task *first_task = NULL;
	    awe_mutex_lock(uxsched->_mutex);

		if(!timer_task_list_empty(&uxsched->_task_list)){
			int64_t now_time = awe_time_ms();
			timer_task* task = timer_task_list_first(&uxsched->_task_list);
			int32_t timeToDelay = (int32_t)(task->_proc_ms - now_time);
			if(timeToDelay <= 0){
				first_task = task;
				timer_task_list_del(&uxsched->_task_list, task); // delete it.
			}
		}

		if(uxsched->_have_rmopt == 1){
			uxsched->_have_rmopt = 0;
			awe_cond_broadcast(uxsched->_cond);
		}
		awe_mutex_unlock(uxsched->_mutex);

		if(first_task != NULL){
			if(first_task->_proc != NULL && first_task->_state == TIMER_TASK_VALID){
				first_task->_proc(first_task->_userdata, first_task->_arg);
			}
			timer_task_destroy(first_task); // destroy task
		}
	}

	awe_mutex_lock(uxsched->_mutex);
	awe_cond_broadcast(uxsched->_cond);
	awe_mutex_unlock(uxsched->_mutex);

	ALOGV("out of thread");
	return AWE_NOERROR;
}

awe_uxscheduler_t* awe_uxscheduler_create(){
	awe_uxscheduler_t *new_uxsched = awe_mallocz(sizeof(awe_uxscheduler_t));
	ALOGV("create(%p).", new_uxsched);

	new_uxsched->_started = false;
	awe_mutex_create(&new_uxsched->_mutex, AWE_MUTEX_DEFAULT);
	awe_cond_create(&new_uxsched->_cond);

	awe_thread_create(&new_uxsched->_thread, uxscheduler_thread_process, new_uxsched);

	new_uxsched->_delayMs = 1000;
	new_uxsched->_have_rmopt = 0;

	INIT_LIST_HEAD(&new_uxsched->_uxevent_list);

	timer_task_list_init(&new_uxsched->_task_list);

	return new_uxsched;
}

static void awe_uxscheduler_clear(awe_uxscheduler_t *uxsched){
	struct list_head *pos = NULL;
	struct list_head *tmp = NULL;

	uxevent *uxe;
	awe_mutex_lock(uxsched->_mutex);
	list_for_each_safe(pos, tmp, &uxsched->_uxevent_list)
	{
		uxe = list_entry(pos, uxevent, _head);

		list_del(pos);
		uxevent_destroy(uxe);
	}
	awe_mutex_unlock(uxsched->_mutex);
}

void awe_uxscheduler_destroy(awe_uxscheduler_t* uxsched){
	awe_uxscheduler_stop(uxsched);
	ALOGV("destroy(%p).", uxsched);

	awe_uxscheduler_clear(uxsched);

	timer_task_list_clear(&uxsched->_task_list);

	if(uxsched->_mutex != NULL){
		awe_mutex_destroy(uxsched->_mutex);
		uxsched->_mutex = NULL;
	}
	if(uxsched->_cond != NULL){
		awe_cond_destroy(uxsched->_cond);
		uxsched->_cond = NULL;
	}
	if(uxsched->_thread != NULL){
		awe_thread_destroy(uxsched->_thread);
		uxsched->_thread = NULL;
	}

	awe_free(uxsched);
}

awe_status_t awe_uxscheduler_start(awe_uxscheduler_t* uxsched, const char* thread_name, int32_t delayMs){
	ALOGV("start(%p)", uxsched);
	awe_mutex_lock(uxsched->_mutex);
	awe_status_t rv = AWE_NOERROR;
	do{
		if(uxsched->_started == true){
			rv = AWE_OK;
			break;
		}
		uxsched->_started = true;

		if(delayMs <= 60000){
			uxsched->_delayMs = delayMs;
		}else{
			uxsched->_delayMs = 1000;
		}

		rv = awe_thread_start(uxsched->_thread, thread_name, AWE_PRIORITY_DEFAULT, 0);
		ALOG_ASSERT(rv != AWE_OK);
	}while(0);
	awe_mutex_unlock(uxsched->_mutex);
	return rv;
}

void awe_uxscheduler_stop(awe_uxscheduler_t* uxsched){
	ALOGV("stop(%p)", uxsched);
	awe_mutex_lock(uxsched->_mutex);
	if(!uxsched->_started){
		awe_mutex_unlock(uxsched->_mutex);
		return ;
	}
	uxsched->_started = false;

	awe_thread_request_exit(uxsched->_thread);

	awe_cond_signal(uxsched->_cond);
	awe_mutex_unlock(uxsched->_mutex);

	awe_mutex_lock(uxsched->_mutex);
	while(awe_thread_running(uxsched->_thread)){
		awe_cond_timedwait(uxsched->_cond, uxsched->_mutex, CONDWAIT_MS(10)); //10 ms
	}
	awe_mutex_unlock(uxsched->_mutex);
}

awe_status_t awe_uxscheduler_add(awe_uxscheduler_t* uxsched, int ufd,
		int conditionSet, sockethandler_proc* proc, void* userdata){
	if(ufd == 0){
		return -1;
	}

	if(conditionSet == 0){
		awe_uxscheduler_rm(uxsched, ufd);
		return AWE_OK;
	}

	awe_mutex_lock(uxsched->_mutex);
	uxevent* uxe = uxscheduler_uxevent_find(&uxsched->_uxevent_list, ufd);
	if(uxe == NULL){
		uxe = uxevent_create(ufd, conditionSet, proc, userdata);
		list_add_tail(&uxe->_head, &uxsched->_uxevent_list);
	}else{
		uxe->_conditionSet = conditionSet;
		uxe->_proc = proc;
		uxe->_userdata = userdata;
	}

	awe_cond_signal(uxsched->_cond);
	awe_mutex_unlock(uxsched->_mutex);
	return AWE_OK;
}

void awe_uxscheduler_disable(awe_uxscheduler_t* uxsched, int ufd, int events){
	awe_mutex_lock(uxsched->_mutex);
	uxevent* uxe = uxscheduler_uxevent_find(&uxsched->_uxevent_list, ufd);
	if(uxe != NULL){
		uxe->_conditionSet &= ~events;
	}
	awe_mutex_unlock(uxsched->_mutex);
}

void awe_uxscheduler_enable(awe_uxscheduler_t* uxsched, int ufd, int events){
	awe_mutex_lock(uxsched->_mutex);
	uxevent* uxe = uxscheduler_uxevent_find(&uxsched->_uxevent_list, ufd);
	if(uxe != NULL){
		uxe->_conditionSet |= events;
	}
	awe_mutex_unlock(uxsched->_mutex);
}

void awe_uxscheduler_rm(awe_uxscheduler_t* uxsched, int ufd){
	awe_mutex_lock(uxsched->_mutex);
	uxevent* uxe = uxscheduler_uxevent_find(&uxsched->_uxevent_list, ufd);
	if(uxe != NULL){
		list_del(&uxe->_head);
		uxevent_destroy(uxe);

		if(uxsched->_started == true && (awe_os_thread_id() != awe_thread_id(uxsched->_thread))){
			uxsched->_have_rmopt = 1;

			awe_cond_timedwait(uxsched->_cond, uxsched->_mutex, CONDWAIT_MS(uxsched->_delayMs));
		}
	}
	awe_mutex_unlock(uxsched->_mutex);
}

awe_uxsched_task_id awe_uxscheduler_post(awe_uxscheduler_t* uxsched,
		int32_t delay_ms, awe_uxscheduler_task_proc* proc, void *userdata, long arg){
	awe_mutex_lock(uxsched->_mutex);
	if(!uxsched->_started){
		awe_mutex_unlock(uxsched->_mutex);
		return NULL;
	}

	timer_task_id new_task = timer_task_list_insert(&uxsched->_task_list, delay_ms, proc, userdata, arg);

	awe_cond_signal(uxsched->_cond);
	awe_mutex_unlock(uxsched->_mutex);

	return (awe_uxsched_task_id)new_task;
}

awe_status_t awe_uxscheduler_cancel(awe_uxscheduler_t* uxsched, awe_uxsched_task_id id){
	awe_status_t found = -1;
	awe_mutex_lock(uxsched->_mutex);
	if(!uxsched->_started){
		awe_mutex_unlock(uxsched->_mutex);
		return found;
	}

	found = timer_task_list_cancel(&uxsched->_task_list, id);
	awe_mutex_unlock(uxsched->_mutex);
	return found;
}
