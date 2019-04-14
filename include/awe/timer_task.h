/*
 * timer_task.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_TIMER_TASK_H_
#define AWE_TIMER_TASK_H_

#include <awe/awe.h>
#include <awe/time.h>
#include <awe/dlist.h>

AWE_BEGIN_DECLS

/**
 * delay task state
 */
typedef enum timer_task_state{
	TIMER_TASK_VALID	= 0, //task valid
	TIMER_TASK_INVALID	= 1, //task invalid
}timer_task_state;

typedef void* timer_task_id;
typedef void timer_task_proc(void *userdata, long arg);

typedef struct timer_task{
	struct list_head	_item; // list item

	int64_t				_proc_ms; // proc time

	timer_task_proc*	_proc;
	void*				_userdata;
	long				_arg;

	volatile int32_t	_state;
}timer_task;

timer_task* timer_task_create(int64_t proc_ms, timer_task_proc* proc, void *userdata, long arg);

void timer_task_destroy(timer_task *task);


typedef struct timer_task_list{
	struct list_head _tasklist; // list head
}timer_task_list;

awe_status_t timer_task_list_init(timer_task_list* list);

int timer_task_list_empty(timer_task_list* list);

timer_task* timer_task_list_first(timer_task_list* list);

timer_task_id timer_task_list_insert(timer_task_list* list,
		int32_t delay_ms, timer_task_proc* proc, void *userdata, long arg);

void timer_task_list_del(timer_task_list* list, timer_task* task);

awe_status_t timer_task_list_cancel(timer_task_list* list, timer_task_id id);

void timer_task_list_clear(timer_task_list* list);

AWE_END_DECLS

#endif /* AWE_TIMER_TASK_H_ */
