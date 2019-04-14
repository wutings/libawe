/*
 * ringbuffer.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_RINGBUFFER_H_
#define AWE_RINGBUFFER_H_

#include <stdint.h>
#include <awe/awe.h>

AWE_BEGIN_DECLS

typedef struct awe_ringbuffer awe_ringbuffer_t;

awe_ringbuffer_t* awe_ringbuffer_create(size_t capacity);
void awe_ringbuffer_destroy(awe_ringbuffer_t* ring);

awe_status_t awe_ringbuffer_moveReadPosition(awe_ringbuffer_t* ring, size_t size);

size_t awe_ringbuffer_get(awe_ringbuffer_t* ring, char* buffer, size_t size);
size_t awe_ringbuffer_put(awe_ringbuffer_t* ring, const char* buffer, size_t size);

size_t awe_ringbuffer_size(awe_ringbuffer_t* ring);
size_t awe_ringbuffer_capacity(awe_ringbuffer_t* ring);

AWE_END_DECLS

#endif /* AWE_RINGBUFFER_H_ */
