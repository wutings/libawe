/*
 * linkedlist.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include <stdlib.h>
#include <string.h>

#define AWE_LOG_TAG "linkedlist"
#include "awe/log.h"

#include "awe/atomic.h"
#include "awe/linkedlist.h"


static awe_linked_obj* linked_obj_create(awe_object *obj){
	awe_linked_obj *e = awe_mallocz(sizeof(awe_linked_obj));
	e->_object = obj;
	return e;
}

static inline void linked_obj_free(awe_linked_obj *obj){
	awe_free(obj);
}

awe_status_t awe_linkedlist_init(awe_linkedlist *list, awe_object_autorelease_proc *proc){
	INIT_LIST_HEAD(&list->_linkedhead);
	list->_release_proc = proc;
	list->_size = 0;
	return AWE_OK;
}

static inline void linkedlist_addtolist(awe_linkedlist *list, awe_linked_obj *obj){
	list_add_tail(&obj->_head, &list->_linkedhead);
	++list->_size;
}

static inline awe_object* linkedlist_rmfromlist(awe_linkedlist *list, awe_linked_obj *obj){
	list_del(&obj->_head);
	--list->_size;

	awe_object *o = obj->_object;

	linked_obj_free(obj);
	return o;
}

static inline void linkedlist_delfromlist(awe_linkedlist *list, awe_linked_obj *obj){
	list_del(&obj->_head);
	--list->_size;

	list->_release_proc(&obj->_object);

	linked_obj_free(obj);
}

awe_status_t awe_linkedlist_add(awe_linkedlist *list, awe_object *obj){
	awe_linked_obj *o = linked_obj_create(obj);
	atomic_fetch_add(&obj->_refs, 1);
	linkedlist_addtolist(list, o);
	return AWE_OK;
}

awe_object* awe_linkedlist_removeFirst(awe_linkedlist *list){
	if(list_empty(&list->_linkedhead)){
		return NULL;
	}
	awe_linked_obj *obj = list_entry(list->_linkedhead.next, awe_linked_obj, _head);
	return linkedlist_rmfromlist(list, obj/*, false*/);
}

awe_object* awe_linkedlist_remove(awe_linkedlist *list,
		bool (*equals)(awe_object *e, void* context), void* context){
	awe_linked_obj *obj = NULL;
	struct list_head *pos = NULL;
	struct list_head *tmp = NULL;
	list_for_each_safe(pos, tmp, &list->_linkedhead)
	{
		obj = list_entry(pos, awe_linked_obj, _head);
		if(equals(obj->_object, context)){
			return linkedlist_rmfromlist(list, obj/*, false*/);
		}
	}
	return NULL;
}

awe_status_t awe_linkedlist_del(awe_linkedlist *list, awe_object *obj){
	awe_linked_obj *o = NULL;

	struct list_head *pos = NULL;
	struct list_head *tmp = NULL;

	list_for_each_safe(pos, tmp, &list->_linkedhead)
	{
		o = list_entry(pos, awe_linked_obj, _head);
		if(o->_object == obj){
			linkedlist_delfromlist(list, o/*, true*/);
			return AWE_OK;
		}
	}
	return -1;
}

awe_status_t awe_linkedlist_del2(awe_linkedlist *list, struct list_head *pos){
	awe_linked_obj *obj = list_entry(pos, awe_linked_obj, _head);
	linkedlist_delfromlist(list, obj/*, true*/);
	return AWE_OK;
}

awe_status_t awe_linkedlist_del3(awe_linkedlist *list,
		bool (*equals)(awe_object *e, void* context), void* context){
	awe_linked_obj *obj = NULL;
	struct list_head *pos = NULL;
	struct list_head *tmp = NULL;
	list_for_each_safe(pos, tmp, &list->_linkedhead)
	{
		obj = list_entry(pos, awe_linked_obj, _head);
		if(equals(obj->_object, context)){
			linkedlist_delfromlist(list, obj/*, true*/);
			return AWE_OK;
		}
	}
	return -1;
}

bool awe_linkedlist_contains(awe_linkedlist *list, awe_object *obj){
	awe_linked_obj *o = NULL;
	struct list_head *pos = NULL;
	list_for_each(pos, &list->_linkedhead)
	{
		o = list_entry(pos, awe_linked_obj, _head);
		if(o->_object == obj){
			return true;
		}
	}
	return false;
}

void awe_linkedlist_clear(awe_linkedlist *list){
	awe_linked_obj *obj = NULL;

	struct list_head *pos = NULL;
	struct list_head *tmp = NULL;

	list_for_each_safe(pos, tmp, &list->_linkedhead)
	{
		obj = list_entry(pos, awe_linked_obj, _head);
		linkedlist_delfromlist(list, obj/*, true*/);
	}
	list->_size = 0;
}

int32_t awe_linkedlist_size(awe_linkedlist *list){
	return list->_size;
}

awe_object* awe_linkedlist_getFirst(awe_linkedlist *list){
	if(list_empty(&list->_linkedhead)){
		return NULL;
	}
	awe_linked_obj *obj = list_entry(list->_linkedhead.next, awe_linked_obj, _head);
	return obj->_object;
}

awe_object* awe_linkedlist_getLast(awe_linkedlist *list){
	if(list_empty(&list->_linkedhead)){
		return NULL;
	}
	awe_linked_obj *obj = list_entry(list->_linkedhead.prev, awe_linked_obj, _head);
	return obj->_object;
}

awe_object* awe_linkedlist_get(awe_linkedlist *list,
		bool (*equals)(awe_object *e, void* context), void* context){
	awe_linked_obj *obj = NULL;
	struct list_head *pos = NULL;
	struct list_head *tmp = NULL;
	list_for_each_safe(pos, tmp, &list->_linkedhead)
	{
		obj = list_entry(pos, awe_linked_obj, _head);
		if(equals(obj->_object, context)){
			return obj->_object;
		}
	}
	return NULL;
}

awe_object* awe_linkedlist_getByIndex(awe_linkedlist *list, int index){
	struct list_head *pos = NULL;
	int i = 0;
	list_for_each(pos, &list->_linkedhead){
		if(i++ == index){
			return list_entry(pos, awe_linked_obj, _head)->_object;
		}
	}
	return NULL;
}

void awe_linkedlist_forEach(awe_linkedlist *list,
        bool (*callback)(struct list_head *pos, awe_object *e, void* context),
        void* context){
	awe_linked_obj *obj = NULL;

	struct list_head *pos = NULL;
	struct list_head *tmp = NULL;

	list_for_each_safe(pos, tmp, &list->_linkedhead)
	{
		obj = list_entry(pos, awe_linked_obj, _head);
		if(!callback(pos, obj->_object, context)){
			return;
		}
	}
}

bool linkedlist_item_equals(awe_object *obj, void* context){
	return obj == (awe_object*)context;
}
