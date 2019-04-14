/*
 * arch_thread_mutex.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_ARCH_THREAD_MUTEX_H_
#define AWE_ARCH_THREAD_MUTEX_H_

struct awe_mutex {
    pthread_mutex_t _mutex;
};

#endif /* AWE_ARCH_THREAD_MUTEX_H_ */
