/*
 * ringarray.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_RINGARRAY_H_
#define AWE_RINGARRAY_H_

#include <stdlib.h>
#include <string.h>
#include <awe/awe.h>


AWE_BEGIN_DECLS

/**
 * ring array free lock
 */
typedef struct awe_ringarray awe_ringarray_t;

awe_ringarray_t* awe_ringarray_create(int32_t block_size, int32_t capacity);

void awe_ringarray_destroy(awe_ringarray_t *ring);

void* awe_ringarray_get(awe_ringarray_t *ring, int index);
void* awe_ringarray_writeReady(awe_ringarray_t *ring, int *wresult);
void awe_ringarray_writeFinish(awe_ringarray_t *ring, int wresult);
void* awe_ringarray_readReady(awe_ringarray_t *ring, int *rresult);
void awe_ringarray_readFinish(awe_ringarray_t *ring, int rresult);
int32_t awe_ringarray_capacity(awe_ringarray_t *ring);
int32_t awe_ringarray_size(awe_ringarray_t *ring);

AWE_END_DECLS

#endif /* AWE_RINGARRAY_H_ */
