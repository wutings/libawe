/*
 * scheduler.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include <ev.h>

#define AWE_LOG_TAG "scheduler"
#include "awe/log.h"

#include "awe/awe.h"
#include "awe/socket_helper.h"
#include "awe/socketpair.h"

#include "awe/scheduler.h"

struct awe_scheduler{
	uint32_t _id;
	volatile awe_bool _started;

	awe_socketpair _sched_sockpair;
	awe_socketpeer* _sched_rpeer;
	awe_socketpeer* _sched_wpeer;

	awe_linkedlist _evio_list;
	awe_linkedlist _evtimer_list;
	awe_linkedlist _evtimeout_list;

	struct ev_loop *_ev_loop;

	awe_thread_t *_thread;

	// token for trigger event
	awe_watchid _triggertoken;
};

typedef enum evtimer_type{
	EVTIMER_NONPERIODIC = 0,
	EVTIMER_PERIODIC = 1,
}evtimer_type;

typedef struct awe_evio_userdata{
	AWE_OBJECT_DEC;

	ev_io _io_w;

	sockethandler_proc *_proc;
	void* _userdata;
}awe_evio_userdata;

typedef struct awe_evtimer_userdata{
	AWE_OBJECT_DEC;

	evtimer_type _type;

	ev_timer _timer_w;

	taskproc *_proc;
	void* _userdata;

	long _arg;
}awe_evtimer_userdata;

typedef struct awe_evtigger{
	taskproc *_proc;
	void* _userdata;

	long _arg;
}awe_evtigger;


static awe_evio_userdata* awe_evio_userdata_create(sockethandler_proc* proc, void* userdata){
	awe_evio_userdata* newdata = (awe_evio_userdata*)awe_mallocz(sizeof(awe_evio_userdata));
//	ALOGV("awe_evio_userdata_create(%p)", newdata);
	awe_object_init(newdata);

	newdata->_io_w.data = newdata;

	newdata->_proc = proc;
	newdata->_userdata = userdata;
	return newdata;
}

static void awe_evio_userdata_autorelease(awe_evio_userdata **evdata){
	if(evdata == NULL || *evdata == NULL){
		return ;
	}

	int32_t refs = awe_object_ref_dec(*evdata);
	if(refs == 1){
//		ALOG_ASSERT(ev_is_active(&(*evdata)->_io_w));//todo

//		ALOGV("awe_evio_userdata_autorelease(%p)", *evdata);
		awe_free(*evdata);
	}
	*evdata = NULL;
}

static awe_evtimer_userdata* awe_evtimer_userdata_create(evtimer_type type, taskproc* proc, void* userdata, long arg){
	awe_evtimer_userdata* newdata = (awe_evtimer_userdata*)awe_mallocz(sizeof(awe_evtimer_userdata));
//	ALOGV("awe_evtimer_userdata_create(%p)", newdata);
	awe_object_init(newdata);
	newdata->_type = type;

	newdata->_timer_w.data = newdata;

	newdata->_proc = proc;
	newdata->_userdata = userdata;
	newdata->_arg = arg;
	return newdata;
}

static void awe_evtimer_userdata_autorelease(awe_evtimer_userdata **evdata){
	if(evdata == NULL || *evdata == NULL){
		return ;
	}

	int32_t refs = awe_object_ref_dec(*evdata);
	if(refs == 1){
//		ALOG_ASSERT(ev_is_active(&(*evdata)->_timer_w));

//		ALOGV("awe_evtimer_userdata_autorelease(%p)", *evdata);
		awe_free(*evdata);
	}
	*evdata = NULL;
}

//---------------------------------------------------------------------

static void scheduler_evio_autorelease_proc(awe_object **e){
	awe_evio_userdata_autorelease((awe_evio_userdata **)e);
}

static void scheduler_evtimer_autorelease_proc(awe_object **e){
	awe_evtimer_userdata_autorelease((awe_evtimer_userdata **)e);
}

static void scheduler_evio_cb(EV_P_ ev_io *w, int events) {
	awe_evio_userdata* evdata = (awe_evio_userdata*)w->data;
	evdata->_proc(evdata->_userdata, w->fd, events);
}

//static void scheduler_timeouttask_proc(void* userdata, long arg){
//	awe_scheduler_t *sched = (awe_scheduler_t *)userdata;
//	awe_evtimer_userdata* evdata = (awe_evtimer_userdata*)awe_linkedlist_removeFirst(&sched->_evtimeout_list);
//	if(evdata != NULL){
//		ev_timer_stop(sched->_ev_loop, evdata->_w);
//		awe_evtimer_userdata_autorelease(&evdata);
//	}
//}

static void scheduler_evtimer_cb(EV_P_ ev_timer *w, int events){
	awe_evtimer_userdata* evdata = (awe_evtimer_userdata*)w->data;

	evdata->_proc(evdata->_userdata, evdata->_arg);

	int type = evdata->_type;
	if(type == EVTIMER_NONPERIODIC){
		awe_scheduler_t* sched = (awe_scheduler_t*)ev_userdata(loop);
//		awe_linkedlist_add(&sched->_evtimeout_list, (awe_object *)evdata);
//		awe_scheduler_triggerevent(sched, scheduler_timeouttask_proc, sched, 0);
		//todo

		awe_linkedlist_del(&sched->_evtimer_list, (awe_object*)evdata);
	}
}

static void scheduler_disable_trigger(awe_scheduler_t *sched);

static void scheduler_trigger_event_proc(void* userdata, int fd, int mask){
	awe_scheduler_t *sched = (awe_scheduler_t *)userdata;

	awe_evtigger trigger;
	int bytesRead = awe_socketpeer_read(fd, &trigger, sizeof(awe_evtigger));
	if(bytesRead > 0){
		taskproc *proc = (taskproc*)trigger._proc;
		if(proc){
			proc(trigger._userdata, trigger._arg);
		}
	}else if(bytesRead < 0){
		scheduler_disable_trigger(sched);
		ALOGE("bytesRead:%d, trigger disable.", bytesRead);
	}
}

static inline int scheduler_trigger_event(awe_scheduler_t *sched, awe_evtigger *trigger){
	return awe_socketpeer_write(sched->_sched_wpeer, trigger, sizeof(awe_evtigger));
}

/*static*/
void scheduler_disable_trigger(awe_scheduler_t *sched){
//	ALOGV("disable_trigger.");
	awe_scheduler_socktoken_destroy(sched, &sched->_triggertoken);
}

static awe_status_t scheduler_enable_trigger(awe_scheduler_t *sched){
//	ALOGV("enable_trigger.");
	sched->_sched_rpeer = awe_socketpair_peer0(&sched->_sched_sockpair);
	sched->_sched_wpeer = awe_socketpair_peer1(&sched->_sched_sockpair);

	sched->_triggertoken = awe_scheduler_watch(sched, awe_socketpeer_fd(sched->_sched_rpeer),
			AWE_READ, scheduler_trigger_event_proc, sched);

	return AWE_OK;
}

static void scheduler_run(awe_scheduler_t* sched){
	ev_run(sched->_ev_loop, 0);
	ALOGI("id:%u, scheduler, interrupt.", sched->_id);
}

static int scheduler_thread_process(awe_thread_t* t, void* userdata){
	awe_scheduler_t* sched = (awe_scheduler_t*)userdata;

	scheduler_run(sched);

//	awe_mutex_lock(chan->_mutex);
//	awe_cond_broadcast(chan->_cond);
//	awe_mutex_unlock(chan->_mutex);
	return AWE_NOERROR;
}

static awe_scheduler_t* awe_scheduler_internal_create(uint32_t id, int flags){
	awe_scheduler_t *new_sched = (awe_scheduler_t *)awe_mallocz(sizeof(awe_scheduler_t));
	ALOGV("create(%p), id:%u.", new_sched, id);
	if(new_sched == NULL){
		return NULL;
	}

	new_sched->_id = id;

	awe_linkedlist_init(&new_sched->_evio_list, scheduler_evio_autorelease_proc);
	awe_linkedlist_init(&new_sched->_evtimer_list, scheduler_evtimer_autorelease_proc);
	awe_linkedlist_init(&new_sched->_evtimeout_list, scheduler_evtimer_autorelease_proc);

	new_sched->_ev_loop = (flags == 5) ? EV_DEFAULT : ev_loop_new(0);
	ev_set_userdata(new_sched->_ev_loop, new_sched);

	AWE_CHECK_EQ(AWE_OK, awe_thread_create(&new_sched->_thread, scheduler_thread_process, new_sched));

	ALOGV("id:%u, loop:%p.", id, new_sched->_ev_loop);
	return new_sched;
}

awe_scheduler_t* awe_scheduler_create(uint32_t id){
	return awe_scheduler_internal_create(id, 0);
}

awe_scheduler_t* awe_scheduler_main_create(uint32_t id){
	return awe_scheduler_internal_create(id, 5);
}

void awe_scheduler_destroy(awe_scheduler_t *sched){
	awe_scheduler_stop(sched);

	if(sched->_ev_loop != NULL){
		ev_loop_destroy(sched->_ev_loop);
		sched->_ev_loop = NULL;
	}

	if(sched->_thread != NULL){
		awe_thread_destroy(sched->_thread);
		sched->_thread = NULL;
	}

	ALOGV("destroy(%p), id:%u.", sched, sched->_id);
	awe_free(sched);
}

awe_status_t awe_scheduler_start(awe_scheduler_t *sched, const char* thread_name, int thread){
	if(sched->_started){
		return AWE_OK;
	}
	do{
		sched->_started = true;

		AWE_CHECK_EQ(AWE_OK, awe_socketpair_init(&sched->_sched_sockpair, SOCK_DGRAM));

		if(scheduler_enable_trigger(sched)){
			ALOGE("enable_trigger error.");
			break;
		}

		if(thread == 1){
			if(awe_thread_start(sched->_thread, thread_name, AWE_PRIORITY_DEFAULT, 0) != AWE_OK){
				break;
			}
			awe_sleep(10);
		}

		ALOGI("id:%u, scheduler, start...", sched->_id);
		return AWE_OK;
	}while(0);

	awe_scheduler_stop(sched);
	return -1;
}

void awe_scheduler_stop(awe_scheduler_t *sched){
	if(!sched->_started){
		return ;
	}

	sched->_started = false;

	awe_linkedlist_clear(&sched->_evio_list);
	awe_linkedlist_clear(&sched->_evtimer_list);
	awe_linkedlist_clear(&sched->_evtimeout_list);

	scheduler_disable_trigger(sched);

	awe_socketpair_deinit(&sched->_sched_sockpair);
	ALOGI("id:%u, scheduler, stopped.", sched->_id);
}

void awe_scheduler_break(awe_scheduler_t *sched){
	if(!sched->_started){
		return ;
	}

	ev_break(sched->_ev_loop, EVBREAK_ALL);
}

os_thread_t awe_scheduler_thread_id(awe_scheduler_t *sched){
	return awe_thread_id(sched->_thread);
}

bool awe_scheduler_running(awe_scheduler_t *sched){
	return awe_thread_running(sched->_thread) == 1;
}

void awe_scheduler_run(awe_scheduler_t *sched){
	if(awe_scheduler_running(sched) == true){
		return;
	}
	scheduler_run(sched);
}

awe_watchid awe_scheduler_watch(awe_scheduler_t *sched, int socketNum, int events,
		sockethandler_proc *handlerProc, void *userdata){
	if(!sched->_started){
		return NULL;
	}
	awe_evio_userdata* evdata = awe_evio_userdata_create(handlerProc, userdata);

	ev_io_init(&evdata->_io_w, scheduler_evio_cb, socketNum, events);
	ev_io_start(sched->_ev_loop, &evdata->_io_w);

	awe_watchid token = (awe_watchid)evdata;
	awe_linkedlist_add(&sched->_evio_list, (awe_object *)evdata);
	awe_evio_userdata_autorelease(&evdata);
	return token;
}

void awe_scheduler_disable(awe_scheduler_t *sched, awe_watchid token, int events){
	if(token == NULL){
		return;
	}
	awe_evio_userdata* evdata = (awe_evio_userdata*)token;
	ev_io *fd_watcher = &evdata->_io_w;

	ev_io_stop(sched->_ev_loop, fd_watcher);

	fd_watcher->events &= ~events;
	if(fd_watcher->events > 0){
		ev_io_start(sched->_ev_loop, fd_watcher);
	}
}

void awe_scheduler_enable(awe_scheduler_t *sched, awe_watchid token, int events){
	if(!sched->_started || token == NULL){
		return;
	}
	awe_evio_userdata* evdata = (awe_evio_userdata*)token;
	ev_io *fd_watcher = &evdata->_io_w;

	ev_io_stop(sched->_ev_loop, fd_watcher);

	fd_watcher->events |= events;
	ev_io_start(sched->_ev_loop, fd_watcher);
}

void awe_scheduler_socktoken_destroy(awe_scheduler_t *sched, awe_watchid *token){
	if(token == NULL || *token == NULL){
		return;
	}
	awe_evio_userdata* evdata = (awe_evio_userdata*)awe_linkedlist_remove(&sched->_evio_list,
			linkedlist_item_equals, *token);

//	ALOG_ASSERT(evdata == NULL);
	if(evdata != NULL){
		ev_io_stop(sched->_ev_loop, &evdata->_io_w);
		awe_evio_userdata_autorelease(&evdata);
	}

	*token = NULL;
}


static awe_taskid awe_scheduler_timertask(awe_scheduler_t *sched, evtimer_type type,
		int32_t ms, taskproc* proc, void* userdata, long arg){
	if(!sched->_started){
		return NULL;
	}
	awe_evtimer_userdata* evdata = awe_evtimer_userdata_create(type, proc, userdata, arg);

	double delay = ms / ((double)1000);
	ev_timer_init(&evdata->_timer_w, scheduler_evtimer_cb, delay, 0);
	ev_timer_start(sched->_ev_loop, &evdata->_timer_w);

	awe_taskid token = (awe_taskid)evdata;
	awe_linkedlist_add(&sched->_evtimer_list, (awe_object *)evdata);
	awe_evtimer_userdata_autorelease(&evdata);
	return token;
}

awe_taskid awe_scheduler_delayedtask(awe_scheduler_t *sched, int32_t ms,
		taskproc* proc, void* userdata, long arg){
	return awe_scheduler_timertask(sched, EVTIMER_NONPERIODIC, ms, proc, userdata, arg);
}

awe_taskid awe_scheduler_periodictask(awe_scheduler_t *sched, int32_t ms,
		taskproc* proc, void* userdata, long arg){
	return awe_scheduler_timertask(sched, EVTIMER_PERIODIC, ms, proc, userdata, arg);
}

awe_status_t awe_scheduler_task_again(awe_scheduler_t *sched, awe_taskid token,
		int32_t ms, long arg){
	if(!sched->_started){
		return -1;
	}

	awe_evtimer_userdata* evdata = (awe_evtimer_userdata*)token;
	if(awe_linkedlist_contains(&sched->_evtimer_list, (awe_object*)evdata)){
		evdata->_arg = arg;

		double delay = ms / ((double)1000);
		ev_timer_stop(sched->_ev_loop, &evdata->_timer_w);
		ev_timer_set(&evdata->_timer_w, delay, 0);
		ev_timer_start(sched->_ev_loop, &evdata->_timer_w);
		return AWE_OK;
	}
	return -1;
}

/**
 *
 */
void awe_scheduler_tasktoken_destroy(awe_scheduler_t *sched, awe_taskid *token){
	if(token == NULL || *token == NULL){
		return;
	}

	awe_evtimer_userdata* evdata = (awe_evtimer_userdata*)awe_linkedlist_remove(&sched->_evtimer_list,
			linkedlist_item_equals, *token);
	if(evdata != NULL){
//		awe_linkedlist_add(&sched->_evtimeout_list, (awe_object *)evdata);
//		awe_scheduler_triggerevent(sched, scheduler_timeouttask_proc, sched, 0);

		ev_timer_stop(sched->_ev_loop, &evdata->_timer_w);
		awe_evtimer_userdata_autorelease(&evdata);
	}

	*token = NULL;
}


awe_status_t awe_scheduler_trigger(awe_scheduler_t *sched, taskproc* proc, void* userdata, long arg){
	if(!sched->_started){
		return -1;
	}

	awe_evtigger trigger;
	trigger._proc = proc;
	trigger._userdata = userdata;
	trigger._arg = arg;

	scheduler_trigger_event(sched, &trigger);
	return AWE_OK;
}
