/*
 * arch_thread_rwlock.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_ARCH_THREAD_RWLOCK_H_
#define AWE_ARCH_THREAD_RWLOCK_H_

struct awe_rwlock {
    pthread_rwlock_t _rwlock;
};

#endif /* AWE_ARCH_THREAD_RWLOCK_H_ */
