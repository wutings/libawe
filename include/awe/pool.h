/*
 * pool.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_POOL_H_
#define AWE_POOL_H_

#include <awe/awe.h>

AWE_BEGIN_DECLS

/**
 * @defgroup awe_pools Memory Pool Functions
 * @ingroup AWE
 * @{
 */

/** The fundamental pool type */
typedef struct awe_pool awe_pool_t;

awe_pool_t* awe_pool_create();
void awe_pool_destroy();

void* awe_pool_alloc(awe_pool_t* pool, size_t size);
void* awe_pool_allocz(awe_pool_t* pool, size_t size);

void awe_pool_free(awe_pool_t* pool, void* mem, size_t size);

AWE_END_DECLS

#endif /* AWE_POOL_H_ */
