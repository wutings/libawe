/*
 * mutex.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_MUTEX_H_
#define AWE_MUTEX_H_

#include <awe/awe.h>

AWE_BEGIN_DECLS

/** Opaque thread-local mutex structure */
typedef struct awe_mutex awe_mutex_t;

#define AWE_MUTEX_DEFAULT  0x0   /**< platform-optimal lock behavior */
#define AWE_MUTEX_NESTED   0x1   /**< enable nested (recursive) locks */
#define AWE_MUTEX_UNNESTED 0x2   /**< disable nested locks */

/**
 * Create and initialize a mutex that can be used to synchronize threads.
 * @param mutex the memory address where the newly created mutex will be
 *        stored.
 * @param flags Or'ed value of:
 * <PRE>
 *           AWE_MUTEX_DEFAULT   platform-optimal lock behavior.
 *           AWE_MUTEX_NESTED    enable nested (recursive) locks.
 *           AWE_MUTEX_UNNESTED  disable nested locks (non-recursive).
 * </PRE>
 * @warning Be cautious in using AWE_THREAD_MUTEX_DEFAULT.  While this is the
 * most optimal mutex based on a given platform's performance characteristics,
 * it will behave as either a nested or an unnested lock.
 */
awe_status_t awe_mutex_create(awe_mutex_t** mutex, unsigned flags);

/**
 * Destroy the mutex and free the memory associated with the lock.
 * @param mutex the mutex to destroy.
 */
void awe_mutex_destroy(awe_mutex_t *mutex);

/**
 * Acquire the lock for the given mutex. If the mutex is already locked,
 * the current thread will be put to sleep until the lock becomes available.
 * @param mutex the mutex on which to acquire the lock.
 */
awe_status_t awe_mutex_lock(awe_mutex_t *mutex);

/**
 * Attempt to acquire the lock for the given mutex. If the mutex has already
 * been acquired, the call returns immediately with AWE_EBUSY. Note: it
 * is important that the AWE_STATUS_IS_EBUSY(s) macro be used to determine
 * if the return value was AWE_EBUSY, for portability reasons.
 * @param mutex the mutex on which to attempt the lock acquiring.
 */
awe_status_t awe_mutex_trylock(awe_mutex_t *mutex);

/**
 * Release the lock for the given mutex.
 * @param mutex the mutex from which to release the lock.
 */
awe_status_t awe_mutex_unlock(awe_mutex_t *mutex);

AWE_END_DECLS

#endif /* AWE_MUTEX_H_ */
