/*
 * awe_object.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_OBJECT_H_
#define AWE_OBJECT_H_

#include <awe/atomic.h>

#define AWE_OBJECT_DEC \
	volatile atomic_int _refs

#define awe_object_init(obj) obj->_refs = 1
#define awe_object_ref_inc(obj) atomic_fetch_add(&(obj)->_refs, 1)
#define awe_object_ref_dec(obj) atomic_fetch_sub(&(obj)->_refs, 1)

typedef struct awe_object{
	AWE_OBJECT_DEC;
}awe_object;

#endif /* AWE_OBJECT_H_ */
