/*
 * thread.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#if defined(HAVE_PRCTL)
#include <sys/prctl.h>
#endif

#define AWE_LOG_TAG "thread"
#include "awe/log.h"

#include "awe/thread.h"

struct awe_thread{
	os_thread_t				_tid;

    awe_thread_process		_process;

    volatile int			_exit_pending;
    volatile int			_running;

    void					*_userdata;
    char					*_thread_name;
};

typedef void* (*awe_pthread_entry)(void*);

void awe_set_thread_name(const char* name) {
#if defined(HAVE_PRCTL)
    // Mac OS doesn't have this, and we build libutil for the host too
    int hasAt = 0;
    int hasDot = 0;
    const char *s = name;
    while (*s) {
        if (*s == '.') hasDot = 1;
        else if (*s == '@') hasAt = 1;
        s++;
    }
    int len = s - name;
    if (len < 15 || hasAt || !hasDot) {
        s = name;
    } else {
        s = name + len - 15;
    }
    prctl(PR_SET_NAME, (unsigned long) s, 0, 0, 0);
#endif
}

static void *dummy_worker(void *opaque){
    awe_thread_t *thread = (awe_thread_t*)opaque;
    if(thread->_thread_name){
    	awe_set_thread_name(thread->_thread_name);
    	free(thread->_thread_name);
    	thread->_thread_name = NULL;
    }

    thread->_process(thread, thread->_userdata);

    thread->_tid = 0;
    thread->_exit_pending = 1;
    thread->_running = 0;
    return NULL;
}

awe_status_t awe_thread_create(awe_thread_t **thread, awe_thread_process process,
																void *userdata){
	awe_thread_t *new_thread = (awe_thread_t *)awe_mallocz(sizeof(awe_thread_t));
    if (new_thread == NULL) {
        return AWE_NO_MEMORY;
    }

    new_thread->_userdata = userdata;
    new_thread->_process = process;
    new_thread->_exit_pending = 1;
    new_thread->_running = 0;

    *thread = new_thread;

    return AWE_OK;
}

void awe_thread_destroy(awe_thread_t *thread){
	awe_free(thread);
}

/*
 * Start the thread in awe_thread_process.
 */
awe_status_t awe_thread_start(awe_thread_t *thread, const char *thread_name, int32_t priority,
        					size_t stack){
	if(thread->_tid != 0){
		return 1;
	}
    thread->_thread_name = thread_name ? strdup(thread_name) : NULL;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (stack) {
        pthread_attr_setstacksize(&attr, stack);
    }

    int result = pthread_create(&thread->_tid, &attr, dummy_worker, thread);
    pthread_attr_destroy(&attr);
    if (result != AWE_OK) {
        ALOGE("awe_thread_start failed (entry=%p, res=%d), "
             "(threadPriority=%d).",
             thread, result, priority/*threadPriority*/);
        return result;
    }

    thread->_exit_pending = 0;
    thread->_running = 1;

    return AWE_OK;
}

os_thread_t awe_thread_id(awe_thread_t *thread){
	return thread->_tid;
}

int awe_thread_exit_pending(awe_thread_t *thread){
	return thread->_exit_pending;
}

int awe_thread_running(awe_thread_t *thread){
	return thread->_running;
}

awe_status_t awe_thread_request_exit(awe_thread_t *thread){
	thread->_exit_pending = 1;
	return AWE_OK;
}

//=================================================
os_thread_t awe_os_thread_id(void){
	return pthread_self();
}

