/*
 * looper.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_LOOPER_H_
#define AWE_LOOPER_H_

#include <awe/awe.h>

AWE_BEGIN_DECLS

typedef struct awe_looper awe_looper_t;

typedef void* awe_looper_task_id;
typedef void awe_looper_task_proc(void *userdata, long arg);

awe_status_t awe_looper_create(awe_looper_t **looper);
void awe_looper_destroy(awe_looper_t *looper);

awe_status_t awe_looper_start(awe_looper_t *looper);
void awe_looper_stop(awe_looper_t *looper);

awe_looper_task_id awe_looper_post(awe_looper_t *looper,
		int32_t delay_ms, awe_looper_task_proc* proc, void *userdata, long arg);

awe_status_t awe_looper_cancel(awe_looper_t *looper, awe_looper_task_id id);

void awe_looper_clear(awe_looper_t *looper);

AWE_END_DECLS

#endif /* AWE_LOOPER_H_ */
