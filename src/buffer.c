/*
 * buffer.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define AWE_LOG_TAG "buffer"
#include "awe/log.h"

#include "awe/buffer.h"

static void awe_buffer_expand(awe_buffer* buffer, int32_t capacity){
	if(buffer->_data != NULL){
		awe_free(buffer->_data);
	}

	buffer->_data = (char*)awe_mallocz(capacity);
	buffer->_capacity = capacity;
}

awe_status_t awe_buffer_init(awe_buffer* buffer, int32_t capacity){
	memset(buffer, 0, sizeof(awe_buffer));
	awe_buffer_expand(buffer, capacity);
	return AWE_OK;
}

void awe_buffer_deinit(awe_buffer* buffer){
	if(buffer->_data != NULL){
		awe_free(buffer->_data);
	}
	memset(buffer, 0, sizeof(awe_buffer));
}

int32_t awe_buffer_capacity(awe_buffer* buffer){
	return buffer->_capacity;
}

char* awe_buffer_base2(awe_buffer* buffer, int32_t expandIfNecessary){
	if(expandIfNecessary > buffer->_capacity){
		awe_buffer_expand(buffer, expandIfNecessary);
	}

	return buffer->_data;
}

