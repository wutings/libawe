/*
 * thread_cond.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <pthread.h>

#define AWE_LOG_TAG "thread_cond"
#include "awe/log.h"

#include "awe/mutex.h"
#include "awe/cond.h"
#include "arch_thread_mutex.h"
#include "arch_thread_cond.h"

awe_status_t awe_cond_create(awe_cond_t** cond){
    awe_cond_t *new_cond;
    awe_status_t rv;

    new_cond = (awe_cond_t *)awe_mallocz(sizeof(awe_cond_t));

    if ((rv = pthread_cond_init(&new_cond->_cond, NULL))) {
#ifdef HAVE_ZOS_PTHREADS
        rv = errno;
#endif
        awe_free(new_cond);
        return rv;
    }

    *cond = new_cond;
    return AWE_OK;
}

void awe_cond_destroy(awe_cond_t *cond){
	pthread_cond_destroy(&cond->_cond);
	awe_free(cond);
}

awe_status_t awe_cond_wait(awe_cond_t *cond, awe_mutex_t *mutex){
    awe_status_t rv;

    rv = pthread_cond_wait(&cond->_cond, &mutex->_mutex);
#ifdef HAVE_ZOS_PTHREADS
    if (rv) {
        rv = errno;
    }
#endif
    return rv;
}

awe_status_t awe_cond_timedwait(awe_cond_t *cond, awe_mutex_t *mutex,
                                                    int64_t timeout){
	awe_status_t rv;
    struct timespec ts;
#if defined(__linux__)
    clock_gettime(CLOCK_REALTIME, &ts);
#else // HAVE_POSIX_CLOCKS
    // we don't support the clocks here.
    struct timeval t;
    gettimeofday(&t, NULL);
    ts.tv_sec = t.tv_sec;
    ts.tv_nsec= t.tv_usec*1000;
#endif // HAVE_POSIX_CLOCKS
    ts.tv_sec += timeout/1000000000;
    ts.tv_nsec+= timeout%1000000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec  += 1;
    }
    rv = pthread_cond_timedwait(&cond->_cond, &mutex->_mutex, &ts);
#ifdef HAVE_ZOS_PTHREADS
    if (rv) {
        rv = errno;
    }
#endif
    return rv;
}

void awe_cond_signal(awe_cond_t *cond){
    /*
     * POSIX says pthread_cond_signal wakes up "one or more" waiting threads.
     * However bionic follows the glibc guarantee which wakes up "exactly one"
     * waiting thread.
     *
     * man 3 pthread_cond_signal
     *   pthread_cond_signal restarts one of the threads that are waiting on
     *   the condition variable cond. If no threads are waiting on cond,
     *   nothing happens. If several threads are waiting on cond, exactly one
     *   is restarted, but it is not specified which.
     */
    pthread_cond_signal(&cond->_cond);
}

void awe_cond_broadcast(awe_cond_t *cond){
	pthread_cond_broadcast(&cond->_cond);
}
