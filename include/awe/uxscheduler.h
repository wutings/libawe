/*
 * uxscheduler.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_UXSCHEDULER_H_
#define AWE_UXSCHEDULER_H_

#include <awe/scheduler.h>

AWE_BEGIN_DECLS

typedef struct awe_uxscheduler awe_uxscheduler_t;

awe_uxscheduler_t* awe_uxscheduler_create();

void awe_uxscheduler_destroy(awe_uxscheduler_t* uxsched);

awe_status_t awe_uxscheduler_start(awe_uxscheduler_t* uxsched, const char* thread_name, int32_t delayMs);
void awe_uxscheduler_stop(awe_uxscheduler_t* uxsched);

//#define UX_READABLE    (1<<0)
//#define UX_WRITABLE    (1<<1)
//#define UX_EXCEPTION   (1<<2)

awe_status_t awe_uxscheduler_add(awe_uxscheduler_t* uxsched, int ufd,
		int conditionSet, sockethandler_proc* proc, void* userdata);

void awe_uxscheduler_disable(awe_uxscheduler_t* uxsched, int ufd, int events);
void awe_uxscheduler_enable(awe_uxscheduler_t* uxsched, int ufd, int events);

void awe_uxscheduler_rm(awe_uxscheduler_t* uxsched, /*UDTSOCKET*/int ufd);

typedef void* awe_uxsched_task_id;
typedef void awe_uxscheduler_task_proc(void *userdata, long arg);

awe_uxsched_task_id awe_uxscheduler_post(awe_uxscheduler_t* uxsched,
		int32_t delay_ms, awe_uxscheduler_task_proc* proc, void *userdata, long arg);

awe_status_t awe_uxscheduler_cancel(awe_uxscheduler_t* uxsched, awe_uxsched_task_id id);

AWE_END_DECLS

#endif /* AWE_UXSCHEDULER_H_ */
