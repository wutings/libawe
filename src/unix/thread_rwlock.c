/*
 * rwlock.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#define AWE_LOG_TAG "pthread_rwlock"
#include "awe/log.h"

#include "awe/rwlock.h"
#include "arch_thread_rwlock.h"

awe_status_t awe_rwlock_create(awe_rwlock_t **rwlock){
	awe_rwlock_t *new_rwlock = NULL;
	awe_status_t rv;

	new_rwlock = (awe_rwlock_t *)awe_mallocz(sizeof(awe_rwlock_t));
    if ((rv = pthread_rwlock_init(&new_rwlock->_rwlock, NULL))) {
#ifdef HAVE_ZOS_PTHREADS
    	rv = errno;
#endif
    	awe_free(new_rwlock);
        return rv;
    }
    *rwlock = new_rwlock;
    return AWE_OK;
}

void awe_rwlock_destroy(awe_rwlock_t *rwlock){
    pthread_rwlock_destroy(&rwlock->_rwlock);
    awe_free(rwlock);
}

awe_status_t awe_rwlock_rdlock(awe_rwlock_t *rwlock){
    awe_status_t rv;

    rv = pthread_rwlock_rdlock(&rwlock->_rwlock);
#ifdef HAVE_ZOS_PTHREADS
    if (rv) {
        rv = errno;
    }
#endif
    return rv;
}

awe_status_t awe_rwlock_tryrdlock(awe_rwlock_t *rwlock){
    awe_status_t rv;

    rv = pthread_rwlock_tryrdlock(&rwlock->_rwlock);
#ifdef HAVE_ZOS_PTHREADS
    if (rv) {
        rv = errno;
    }
#endif
    return rv;
}

awe_status_t awe_rwlock_wrlock(awe_rwlock_t *rwlock){
    awe_status_t rv;

    rv = pthread_rwlock_wrlock(&rwlock->_rwlock);
#ifdef HAVE_ZOS_PTHREADS
    if (rv) {
        rv = errno;
    }
#endif
    return rv;
}

awe_status_t awe_rwlock_trywrlock(awe_rwlock_t *rwlock){
    awe_status_t rv;

    rv = pthread_rwlock_trywrlock(&rwlock->_rwlock);
#ifdef HAVE_ZOS_PTHREADS
    if (rv) {
        rv = errno;
    }
#endif
    return rv;
}

awe_status_t awe_rwlock_unlock(awe_rwlock_t *rwlock){
    awe_status_t rv;

    rv = pthread_rwlock_unlock(&rwlock->_rwlock);
#ifdef HAVE_ZOS_PTHREADS
    if (rv) {
        rv = errno;
    }
#endif
    return rv;
}
