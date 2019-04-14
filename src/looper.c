/*
 * looper.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */
#include <stdlib.h>
#include <string.h>

#define AWE_LOG_TAG "looper"
#include "awe/log.h"

#include "awe/time.h"
#include "awe/dlist.h"
#include "awe/thread.h"
#include "awe/mutex.h"
#include "awe/cond.h"

#include "awe/looper.h"
#include "awe/timer_task.h"

struct awe_looper{
	awe_mutex_t		*_mutex;
	awe_cond_t		*_cond;
	awe_thread_t	*_thread;

	timer_task_list _task_list;

	volatile int	_started;
};

//-------------------------------------------------------------------------------------

static int looper_thread_process(awe_thread_t *t, void *userdata){
	awe_looper_t *looper = (awe_looper_t *)userdata;

	while(!awe_thread_exit_pending(t)){
		awe_mutex_lock(looper->_mutex);
		while(looper->_started && timer_task_list_empty(&looper->_task_list)){
			awe_cond_wait(looper->_cond, looper->_mutex);
		}
		awe_mutex_unlock(looper->_mutex);

		if(!looper->_started){
			break;
		}

		timer_task *first_task = NULL;

		awe_mutex_lock(looper->_mutex);

		int64_t now_time = awe_time_ms();
		timer_task* task = timer_task_list_first(&looper->_task_list);

		int64_t delay_ms = task->_proc_ms - now_time;
		if(delay_ms <= 0){
			first_task = task;
			timer_task_list_del(&looper->_task_list, task); // delete it.
		}else{
			awe_cond_timedwait(looper->_cond, looper->_mutex, CONDWAIT_MS(delay_ms)); //delay_ms ms
		}
		awe_mutex_unlock(looper->_mutex);

		if(first_task != NULL){
			if(first_task->_state == TIMER_TASK_VALID){
				first_task->_proc(first_task->_userdata, first_task->_arg);
			}
			timer_task_destroy(first_task); // destroy task
		}
	}

	awe_mutex_lock(looper->_mutex);
	awe_cond_broadcast(looper->_cond);
	awe_mutex_unlock(looper->_mutex);
	return AWE_NOERROR;
}

awe_status_t awe_looper_create(awe_looper_t **looper){
	awe_looper_t *new_looper = awe_mallocz(sizeof(awe_looper_t));
	ALOGV("create(%p).", new_looper);
	if(new_looper == NULL){
		return AWE_NO_MEMORY;
	}

	awe_mutex_create(&new_looper->_mutex, AWE_MUTEX_DEFAULT);
	awe_cond_create(&new_looper->_cond);
	awe_thread_create(&new_looper->_thread, looper_thread_process, new_looper);

	timer_task_list_init(&new_looper->_task_list);

	*looper = new_looper;
	return AWE_OK;
}

void awe_looper_destroy(awe_looper_t *looper){
	ALOGV("destroy(%p).", looper);
	awe_looper_stop(looper);

	awe_looper_clear(looper);

	if(looper->_mutex != NULL){
		awe_mutex_destroy(looper->_mutex);
		looper->_mutex = NULL;
	}
	if(looper->_cond != NULL){
		awe_cond_destroy(looper->_cond);
		looper->_cond = NULL;
	}
	if(looper->_thread != NULL){
		awe_thread_destroy(looper->_thread);
		looper->_thread = NULL;
	}

	awe_free(looper);
}

awe_status_t awe_looper_start(awe_looper_t *looper){
	awe_mutex_lock(looper->_mutex);
	awe_status_t rv = AWE_NOERROR;
	do{
		if(looper->_started){
			rv = AWE_OK;
			break;
		}
		rv = awe_thread_start(looper->_thread, "awe_looper", AWE_PRIORITY_DEFAULT, 0);
		ALOG_ASSERT(rv != AWE_OK);

		looper->_started = 1;
	}while(0);
	awe_mutex_unlock(looper->_mutex);
	return rv;
}

void awe_looper_stop(awe_looper_t *looper){
	awe_mutex_lock(looper->_mutex);
	if(!looper->_started){
		awe_mutex_unlock(looper->_mutex);
		return ;
	}
	looper->_started = 0;
	awe_cond_signal(looper->_cond);
	awe_mutex_unlock(looper->_mutex);

	AWE_CHECK_NE(awe_os_thread_id(), awe_thread_id(looper->_thread));

	awe_thread_request_exit(looper->_thread);

	awe_mutex_lock(looper->_mutex);
	while(awe_thread_running(looper->_thread)){
		awe_cond_timedwait(looper->_cond, looper->_mutex, CONDWAIT_MS(10)); //10 ms
	}
	awe_mutex_unlock(looper->_mutex);
}

awe_looper_task_id awe_looper_post(awe_looper_t *looper,
		int32_t delay_ms, awe_looper_task_proc* proc, void *userdata, long arg){
	awe_mutex_lock(looper->_mutex);
	if(!looper->_started){
		awe_mutex_unlock(looper->_mutex);
		return NULL;
	}

	timer_task_id new_task = timer_task_list_insert(&looper->_task_list, delay_ms, proc, userdata, arg);

	awe_cond_signal(looper->_cond);
	awe_mutex_unlock(looper->_mutex);

	return (awe_looper_task_id)new_task;
}

awe_status_t awe_looper_cancel(awe_looper_t *looper, awe_looper_task_id id){
	awe_status_t found = -1;
	awe_mutex_lock(looper->_mutex);
	if(!looper->_started){
		awe_mutex_unlock(looper->_mutex);
		return found;
	}

	found = timer_task_list_cancel(&looper->_task_list, id);
	awe_mutex_unlock(looper->_mutex);

	return found;
}

void awe_looper_clear(awe_looper_t *looper){
	awe_mutex_lock(looper->_mutex);
	timer_task_list_clear(&looper->_task_list);
	awe_mutex_unlock(looper->_mutex);
}

