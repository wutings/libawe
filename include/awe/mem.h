/*
 * mem.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_MEM_H_
#define AWE_MEM_H_

#include <awe/awe.h>

AWE_BEGIN_DECLS

void* awe_malloc(size_t size);
void* awe_mallocz(size_t size);

void awe_free(void* mem);

AWE_END_DECLS

#endif /* AWE_MEM_H_ */
