/*
 * linkedlist.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_LINKEDLIST_H_
#define AWE_LINKEDLIST_H_

#include <stdint.h>

#include <awe/awe.h>

AWE_BEGIN_DECLS

typedef void awe_object_autorelease_proc(awe_object **e);

typedef struct awe_linkedlist{
	struct list_head _linkedhead; //list head

	awe_object_autorelease_proc *_release_proc;

	int32_t _size;
}awe_linkedlist;

typedef struct awe_linked_obj{
	struct list_head _head; //list entry
	awe_object *_object;
}awe_linked_obj;

awe_status_t awe_linkedlist_init(awe_linkedlist *list, awe_object_autorelease_proc *proc);

awe_status_t awe_linkedlist_add(awe_linkedlist *list, awe_object *obj);

awe_object* awe_linkedlist_removeFirst(awe_linkedlist *list);
awe_object* awe_linkedlist_remove(awe_linkedlist *list,
		bool (*equals)(awe_object *e, void* context), void* context);

awe_status_t awe_linkedlist_del(awe_linkedlist *list, awe_object *obj);

awe_status_t awe_linkedlist_del2(awe_linkedlist *list, struct list_head *pos);

awe_status_t awe_linkedlist_del3(awe_linkedlist *list,
		bool (*equals)(awe_object *e, void* context), void* context);

bool awe_linkedlist_contains(awe_linkedlist *list, awe_object *obj);

void awe_linkedlist_clear(awe_linkedlist *list);

int32_t awe_linkedlist_size(awe_linkedlist *list);

awe_object* awe_linkedlist_getFirst(awe_linkedlist *list);
awe_object* awe_linkedlist_getLast(awe_linkedlist *list);

awe_object* awe_linkedlist_get(awe_linkedlist *list,
		bool (*equals)(awe_object *e, void* context), void* context);

awe_object* awe_linkedlist_getByIndex(awe_linkedlist *list, int index);

void awe_linkedlist_forEach(awe_linkedlist *list,
        bool (*callback)(struct list_head *pos, awe_object *e, void* context),
        void* context);

#define awe_linkedlist_for(pos, list)		\
	struct list_head* _##pos = NULL;				\
	struct list_head* _##pos##tmp = NULL;			\
	list_for_each_safe(_##pos, _##pos##tmp, &(list)->_linkedhead)

//not safe
#define awe_linkedlist_for_prev(pos, list)		\
		struct list_head* _##pos = NULL;		\
		list_for_each_prev(_##pos, &(list)->_linkedhead)

#define awe_linkedlist_entry(pos)					\
	(list_entry(_##pos, awe_linked_obj, _head)->_object)

#define awe_linkedlist_del_pos(list, pos)	\
		awe_linkedlist_del2(list, _##pos)

//---------------------------------------------------------------------
bool linkedlist_item_equals(awe_object *obj, void* context);


AWE_END_DECLS

#endif /* AWE_LINKEDLIST_H_ */
