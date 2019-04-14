/*
 * ringarray.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#define AWE_LOG_TAG "ringarray"
#include "awe/log.h"

#include "awe/awe.h"
#include "awe/ringarray.h"

//-----------------------------------------------------------
/**
 * ring array free lock
 */
struct awe_ringarray{
	char *_array;
	int32_t _block_size;
	int32_t _capacity;
	int32_t _rindex;
	int32_t _windex;
	int32_t _nmemb;
};

awe_ringarray_t* awe_ringarray_create(int32_t block_size, int32_t capacity){
	ALOG_ASSERT(block_size <= 0 || capacity <= 0);
	awe_ringarray_t* ring = (awe_ringarray_t*)awe_mallocz(sizeof(awe_ringarray_t));
	memset(ring, 0, sizeof(awe_ringarray_t));
	ring->_array = (char *)awe_mallocz(block_size * capacity);
	ring->_block_size = block_size;
	ring->_capacity = capacity;
	return ring;
}

void awe_ringarray_destroy(awe_ringarray_t *ring){
	awe_free(ring->_array);
	awe_free(ring);
}

//int32_t awe_ringarray_init(awe_ringarray_t *ring, int32_t block_size, int32_t capacity){
//	ALOG_ASSERT(block_size <= 0 || capacity <= 0);
//	ALOG_ASSERT(ring->_array != NULL);
//	ring->_array = (char *)awe_mallocz(block_size * capacity);
//	ring->_block_size = block_size;
//	ring->_capacity = capacity;
//	return 0;
//}
void* awe_ringarray_get(awe_ringarray_t *ring, int index){
	return &(ring->_array[index * ring->_block_size]);
}
void* awe_ringarray_writeReady(awe_ringarray_t *ring, int *wresult){
	if(ring->_nmemb >= ring->_capacity){
		*wresult = -2;
		return NULL;
	}
	*wresult = ring->_windex;
	return &(ring->_array[ring->_windex * ring->_block_size]);
}
void awe_ringarray_writeFinish(awe_ringarray_t *ring, int wresult){
	if(wresult < 0){
		return;
	}
	int32_t capacity = ring->_capacity;
	int32_t index = ring->_windex;
	ring->_windex = ++index % capacity;
	++ring->_nmemb;
}
void* awe_ringarray_readReady(awe_ringarray_t *ring, int *rresult){
	if(ring->_nmemb <= 0){
		*rresult = -2;
		return NULL;
	}
	*rresult = ring->_rindex;
	return &(ring->_array[ring->_rindex * ring->_block_size]);
}
void awe_ringarray_readFinish(awe_ringarray_t *ring, int rresult){
	if(rresult <= 0){
		return;
	}
	int32_t capacity = ring->_capacity;
	int32_t index = ring->_rindex;
	ring->_rindex = ++index % capacity;
	--ring->_nmemb;
}
int32_t awe_ringarray_capacity(awe_ringarray_t *ring){
	return ring->_capacity;
}
int32_t awe_ringarray_size(awe_ringarray_t *ring){
	return ring->_nmemb;
}
