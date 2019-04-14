/*
 * rwlock.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_RWLOCK_H_
#define AWE_RWLOCK_H_

#include <awe/awe.h>

AWE_BEGIN_DECLS

/** Opaque read-write thread-safe lock. */
typedef struct awe_rwlock awe_rwlock_t;

/**
 * Create and initialize a read-write lock that can be used to synchronize
 * threads.
 * @param rwlock the memory address where the newly created readwrite lock
 *        will be stored.
 */
awe_status_t awe_rwlock_create(awe_rwlock_t** rwlock);

/**
 * Destroy the read-write lock and free the associated memory.
 * @param rwlock the rwlock to destroy.
 */
void awe_rwlock_destroy(awe_rwlock_t *rwlock);

/**
 * Acquire a shared-read lock on the given read-write lock. This will allow
 * multiple threads to enter the same critical section while they have acquired
 * the read lock.
 * @param rwlock the read-write lock on which to acquire the shared read.
 */
awe_status_t awe_rwlock_rdlock(awe_rwlock_t *rwlock);

/**
 * Attempt to acquire the shared-read lock on the given read-write lock. This
 * is the same as awe_rwlock_rdlock(), only that the function fails
 * if there is another thread holding the write lock, or if there are any
 * write threads blocking on the lock. If the function fails for this case,
 * AWE_EBUSY will be returned. Note: it is important that the
 * AWE_STATUS_IS_EBUSY(s) macro be used to determine if the return value was
 * AWE_EBUSY, for portability reasons.
 * @param rwlock the rwlock on which to attempt the shared read.
 */
awe_status_t awe_rwlock_tryrdlock(awe_rwlock_t *rwlock);

/**
 * Acquire an exclusive-write lock on the given read-write lock. This will
 * allow only one single thread to enter the critical sections. If there
 * are any threads currently holding the read-lock, this thread is put to
 * sleep until it can have exclusive access to the lock.
 * @param rwlock the read-write lock on which to acquire the exclusive write.
 */
awe_status_t awe_rwlock_wrlock(awe_rwlock_t *rwlock);

/**
 * Attempt to acquire the exclusive-write lock on the given read-write lock.
 * This is the same as awe_rwlock_wrlock(), only that the function fails
 * if there is any other thread holding the lock (for reading or writing),
 * in which case the function will return AWE_EBUSY. Note: it is important
 * that the AWE_STATUS_IS_EBUSY(s) macro be used to determine if the return
 * value was AWE_EBUSY, for portability reasons.
 * @param rwlock the rwlock on which to attempt the exclusive write.
 */
awe_status_t awe_rwlock_trywrlock(awe_rwlock_t *rwlock);

/**
 * Release either the read or write lock currently held by the calling thread
 * associated with the given read-write lock.
 * @param rwlock the read-write lock to be released (unlocked).
 */
awe_status_t awe_rwlock_unlock(awe_rwlock_t *rwlock);

AWE_END_DECLS

#endif /* AWE_RWLOCK_H_ */
