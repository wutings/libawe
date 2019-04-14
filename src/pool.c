/*
 * pool.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include "awe/pool.h"

struct awe_pool{
	volatile size_t increment_size;
};

awe_pool_t* awe_pool_create(){
	awe_pool_t* pool = awe_malloc(sizeof(awe_pool_t));
	atomic_init(&pool->increment_size, 0);
	return pool;
}
void awe_pool_destroy(awe_pool_t* pool){
	awe_free(pool);
}

void* awe_pool_alloc(awe_pool_t* pool, size_t size){
    void *ptr = awe_malloc(size);
    if (ptr){
    	atomic_fetch_add(&pool->increment_size, size);
    }
    return ptr;
}
void* awe_pool_allocz(awe_pool_t* pool, size_t size){
    void *ptr = awe_malloc(size);
    if (ptr){
        memset(ptr, 0, size);
        atomic_fetch_add(&pool->increment_size, size);
    }
    return ptr;
}

void awe_pool_free(awe_pool_t* pool, void* mem, size_t size){
	awe_free(mem);
	atomic_fetch_sub(&pool->increment_size, size);
}

