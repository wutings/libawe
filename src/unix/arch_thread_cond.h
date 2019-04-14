/*
 * arch_thread_cond.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_ARCH_THREAD_COND_H_
#define AWE_ARCH_THREAD_COND_H_

struct awe_cond {
	pthread_cond_t _cond;
};

#endif /* AWE_ARCH_THREAD_COND_H_ */
