/*
 * scheduler.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_SCHEDULER_H_
#define AWE_SCHEDULER_H_

#include <awe/awe.h>

AWE_BEGIN_DECLS

typedef struct awe_scheduler awe_scheduler_t;

awe_scheduler_t* awe_scheduler_main_create(uint32_t id);

awe_scheduler_t* awe_scheduler_create(uint32_t id);
void awe_scheduler_destroy(awe_scheduler_t *sched);

awe_status_t awe_scheduler_start(awe_scheduler_t *sched, const char* thread_name, int thread);
void awe_scheduler_stop(awe_scheduler_t *sched);

void awe_scheduler_break(awe_scheduler_t *sched);

os_thread_t awe_scheduler_thread_id(awe_scheduler_t *sched);

bool awe_scheduler_running(awe_scheduler_t *sched);

void awe_scheduler_run(awe_scheduler_t *sched);

#define AWE_READ	0x01 /* io detected read will not block */
#define AWE_WRITE	0x02 /* io detected write will not block */

typedef void* awe_watchid;

// For handling socket operations in the background (from the event loop):
typedef void sockethandler_proc(void* userdata, int fd, int mask);

awe_watchid awe_scheduler_watch(awe_scheduler_t *sched, int socketNum,
		int events, sockethandler_proc* handlerProc, void* userdata);

void awe_scheduler_disable(awe_scheduler_t *sched, awe_watchid token, int events);
void awe_scheduler_enable(awe_scheduler_t *sched, awe_watchid token, int events);

void awe_scheduler_socktoken_destroy(awe_scheduler_t *sched, awe_watchid *token);

typedef void* awe_taskid;
typedef void taskproc(void* userdata, long arg);

/* ms: milliseconds */
awe_taskid awe_scheduler_delayedtask(awe_scheduler_t *sched, int32_t ms,
		taskproc* proc, void* userdata, long arg);
awe_taskid awe_scheduler_periodictask(awe_scheduler_t *sched, int32_t ms,
		taskproc* proc, void* userdata, long arg);
awe_status_t awe_scheduler_task_again(awe_scheduler_t *sched, awe_taskid token,
		int32_t ms, long arg);

void awe_scheduler_tasktoken_destroy(awe_scheduler_t *sched, awe_taskid *token);

awe_status_t awe_scheduler_trigger(awe_scheduler_t *sched, taskproc* proc, void* userdata, long arg);

AWE_END_DECLS

#endif /* AWE_SCHEDULER_H_ */
