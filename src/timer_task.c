/*
 * timer_task.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#define AWE_LOG_TAG "timer_task"
#include "awe/log.h"

#include "awe/timer_task.h"

timer_task* timer_task_create(int64_t proc_ms, timer_task_proc* proc, void *userdata, long arg){
	timer_task *new_task = (timer_task *)awe_mallocz(sizeof(timer_task));
//	ALOGV("create(%p)", new_task);
	new_task->_proc_ms = proc_ms;

	new_task->_proc = proc;
	new_task->_userdata = userdata;

	new_task->_arg = arg;

	new_task->_state = TIMER_TASK_VALID;
	return new_task;
}

void timer_task_destroy(timer_task *task){
//	ALOGV("destroy(%p)", task);
	awe_free(task);
}

//------------------------------------------------------
awe_status_t timer_task_list_init(timer_task_list* list){
	INIT_LIST_HEAD(&list->_tasklist);
	return AWE_OK;
}

int timer_task_list_empty(timer_task_list* list){
	return list_empty(&list->_tasklist);
}

timer_task* timer_task_list_first(timer_task_list* list){
	return list_entry(list->_tasklist.next, timer_task, _item);
}

timer_task_id timer_task_list_insert(timer_task_list* list, int32_t delay_ms,
		timer_task_proc* proc, void *userdata, long arg){
	int64_t now_time = awe_time_ms();
	int64_t proc_time = now_time + delay_ms;
	timer_task *new_task = timer_task_create(proc_time, proc, userdata, arg);

	struct list_head *pos = NULL;
	struct list_head *pos_insert = list->_tasklist.next;

	list_for_each_prev(pos, &list->_tasklist)
	{
		timer_task *task = list_entry(pos, timer_task, _item);
		if(new_task->_proc_ms >= task->_proc_ms){
			pos_insert = task->_item.next;
			break;
		}
	}
	list_add_tail(&new_task->_item, pos_insert);

	return (timer_task_id)new_task;
}

void timer_task_list_del(timer_task_list* list, timer_task* task){
	list_del(&task->_item);
}

awe_status_t timer_task_list_cancel(timer_task_list* list, timer_task_id id){
	awe_status_t found = -1;

	timer_task *task = NULL;
	struct list_head *pos = NULL;

	list_for_each_prev(pos, &list->_tasklist)
	{
		task = list_entry(pos, timer_task, _item);
		if((timer_task *)id == task){
			task->_state = TIMER_TASK_INVALID;
			found = AWE_OK;
			break;
		}
	}

	return found;
}

void timer_task_list_clear(timer_task_list* list){
	timer_task *task = NULL;

	struct list_head *pos = NULL;
	struct list_head *tmp = NULL;

	list_for_each_safe(pos, tmp, &list->_tasklist)
	{
		task = list_entry(pos, timer_task, _item);
		list_del(pos);
		timer_task_destroy(task);
	}
}
