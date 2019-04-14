/*
 * ringbuffer.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include <string.h>

#define AWE_LOG_TAG "ringbuffer"
#include "awe/log.h"

#include "awe/ringbuffer.h"

#define _is_power_of_2_(x) ((x)!=0 && (((x)&((x)-1)) == 0))
#define _min_(a, b) (((a)<(b)) ? (a) : (b))

struct awe_ringbuffer{
	char*	_buffer;
	size_t	_capacity;

	size_t	_in;
	size_t	_out;
};

awe_ringbuffer_t* awe_ringbuffer_create(size_t capacity){
	ALOG_ASSERT(!_is_power_of_2_(capacity));

	awe_ringbuffer_t* ringbuffer = awe_mallocz(sizeof(awe_ringbuffer_t));

	ringbuffer->_buffer = (char*)awe_malloc(capacity);
	ringbuffer->_capacity = capacity;
	ringbuffer->_in = 0;
	ringbuffer->_out = 0;
	return ringbuffer;
}

void awe_ringbuffer_destroy(awe_ringbuffer_t* ring){
	if(ring->_buffer != NULL){
		awe_free(ring->_buffer);
		ring->_buffer = NULL;
	}
	awe_free(ring);
}

awe_status_t awe_ringbuffer_moveReadPosition(awe_ringbuffer_t* ring, size_t size){
	ring->_out += size;
	return AWE_OK;
}

size_t awe_ringbuffer_get(awe_ringbuffer_t* ring, char* buffer, size_t size){
	size_t len = 0;
	size = _min_(size, ring->_in - ring->_out);
	/* first get the data from fifo->out until the end of the buffer */
	len = _min_(size, ring->_capacity -(ring->_out & (ring->_capacity -1)));
	memcpy(buffer, ring->_buffer+(ring->_out & (ring->_capacity -1)), len);
	/* then get the rest (if any) from the beginning of the buffer */
	memcpy(buffer+len, ring->_buffer, size-len);
	ring->_out += size;
	return size;
}

size_t awe_ringbuffer_put(awe_ringbuffer_t* ring, const char* buffer, size_t size){
	size_t len = 0;
	size = _min_(size, ring->_capacity - ring->_in + ring->_out);
	/* first put the data starting from fifo->in to buffer end */
	len = _min_(size, ring->_capacity - (ring->_in & (ring->_capacity-1)));
	memcpy(ring->_buffer + (ring->_in & (ring->_capacity-1)), buffer, len);
	/* then put the rest (if any) at the beginning of the buffer */
	memcpy(ring->_buffer, buffer+len, size-len);
	ring->_in += size;
	return size;
}

size_t awe_ringbuffer_size(awe_ringbuffer_t* ring){
	return (ring->_in - ring->_out);
}

size_t awe_ringbuffer_capacity(awe_ringbuffer_t* ring){
	return ring->_capacity;
}

////--------------------
//ARingBuffer::ARingBuffer(size_t size)
//	:mRingBuffer(new ARingBufferFreeLock(size)){
//}
//
//ARingBuffer::~ARingBuffer(){
//}
//
//status_t ARingBuffer::moveReadPosition(int32_t size){
//	Mutex::Autolock autoLock(mLock);
//	return mRingBuffer->moveReadPosition(size);
//}
//
//size_t ARingBuffer::get(char* buffer, size_t size){
//	Mutex::Autolock autoLock(mLock);
//	return mRingBuffer->get(buffer, size);
//}
//
//size_t ARingBuffer::put(const char* buffer, size_t size){
//	Mutex::Autolock autoLock(mLock);
//	return mRingBuffer->put(buffer, size);
//}
//
//size_t ARingBuffer::size(){
//	Mutex::Autolock autoLock(mLock);
//	return mRingBuffer->size();
//}
//
//size_t ARingBuffer::capacity(){
//	return mRingBuffer->capacity();
//}
