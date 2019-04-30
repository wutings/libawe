/*
 * buffer.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_UTILS_H_
#define AWE_UTILS_H_

#include <stdlib.h>
#include <stdint.h>


AWE_BEGIN_DECLS

typedef struct awe_buffer{
	char* _data;
	int32_t _capacity;
}awe_buffer;

awe_status_t awe_buffer_init(awe_buffer* buffer, int32_t capacity);
void awe_buffer_deinit(awe_buffer* buffer);

int32_t awe_buffer_capacity(awe_buffer* buffer);

char* awe_buffer_base(awe_buffer* buffer);
char* awe_buffer_base2(awe_buffer* buffer, int32_t expandIfNecessary);

AWE_END_DECLS

#endif /* AWE_UTILS_H_ */
