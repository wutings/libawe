/*
 * thread_mutex.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>
#define AWE_LOG_TAG "pthread_mutex"
#include "awe/log.h"

#include "awe/mutex.h"
#include "arch_thread_mutex.h"

awe_status_t awe_mutex_create(awe_mutex_t **mutex, unsigned flags) {
	awe_mutex_t *new_mutex;
	awe_status_t rv;

	new_mutex = (awe_mutex_t *)awe_mallocz(sizeof(awe_mutex_t));

//#ifdef HAVE_PTHREAD_MUTEX_RECURSIVE
	if (flags & AWE_MUTEX_NESTED) {
		pthread_mutexattr_t mattr;

		rv = pthread_mutexattr_init(&mattr);
		if (rv) return rv;

		rv = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
		if (rv) {
			pthread_mutexattr_destroy(&mattr);
			return rv;
		}

		rv = pthread_mutex_init(&new_mutex->_mutex, &mattr);
		pthread_mutexattr_destroy(&mattr);
	} else
//#endif
	rv = pthread_mutex_init(&new_mutex->_mutex, NULL);

	if (rv) {
#ifdef HAVE_ZOS_PTHREADS
		rv = errno;
#endif
		awe_free(new_mutex);
		return rv;
	}

///	awe_pool_cleanup_register(new_mutex->pool, new_mutex, thread_mutex_cleanup,
//			awe_pool_cleanup_null);

	*mutex = new_mutex;
	return AWE_OK;
}

void awe_mutex_destroy(awe_mutex_t *mutex) {
    pthread_mutex_destroy(&mutex->_mutex);
    awe_free(mutex);
}

awe_status_t awe_mutex_lock(awe_mutex_t *mutex) {
	awe_status_t rv;

	rv = pthread_mutex_lock(&mutex->_mutex);
#ifdef HAVE_ZOS_PTHREADS
	if (rv) {
		rv = errno;
	}
#endif

	return rv;
}

awe_status_t awe_mutex_trylock(awe_mutex_t *mutex) {
	awe_status_t rv;

	rv = pthread_mutex_trylock(&mutex->_mutex);
	if (rv) {
#ifdef HAVE_ZOS_PTHREADS
		rv = errno;
#endif
		return (rv == EBUSY) ? AWE_EBUSY : rv;
	}

	return rv;
}

awe_status_t awe_mutex_unlock(awe_mutex_t *mutex) {
	awe_status_t status;

	status = pthread_mutex_unlock(&mutex->_mutex);
#ifdef HAVE_ZOS_PTHREADS
	if (status) {
		status = errno;
	}
#endif

	return status;
}
