/*
 * thread.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_THREAD_H_
#define AWE_THREAD_H_

#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#include <process.h>

/* Windows */
typedef unsigned long int		os_thread_t;

#else
/* Unix */
#include <pthread.h>

typedef pthread_t	os_thread_t;      /**< native thread */

#endif

#include <awe/awe.h>

AWE_BEGIN_DECLS

enum {
    /*
     * Use the levels below when appropriate. Intermediate values are
     * acceptable, preferably use the {MORE|LESS}_FAVORABLE constants below.
     */
    AWE_PRIORITY_LOWEST         =  19,

    /* use for background tasks */
    AWE_PRIORITY_BACKGROUND     =  10,

    /* most threads run at normal priority */
    AWE_PRIORITY_NORMAL         =   0,

    /* threads currently running a UI that the user is interacting with */
    AWE_PRIORITY_FOREGROUND     =  -2,

    /* the main UI thread has a slightly more favorable priority */
    AWE_PRIORITY_DISPLAY        =  -4,

    /* ui service treads might want to run at a urgent display (uncommon) */
    AWE_PRIORITY_URGENT_DISPLAY =  -8, /*HAL_PRIORITY_URGENT_DISPLAY,*/

    /* all normal audio threads */
    AWE_PRIORITY_AUDIO          = -16,

    /* service audio threads (uncommon) */
    AWE_PRIORITY_URGENT_AUDIO   = -19,

    /* should never be used in practice. regular process might not
     * be allowed to use this level */
    AWE_PRIORITY_HIGHEST        = -20,

    AWE_PRIORITY_DEFAULT        = AWE_PRIORITY_NORMAL,
    AWE_PRIORITY_MORE_FAVORABLE = -1,
    AWE_PRIORITY_LESS_FAVORABLE = +1,
};

typedef struct awe_thread awe_thread_t;
typedef int (*awe_thread_process)(awe_thread_t *gthread, void *userdata);

/**
 * Create a new thread of execution
 * @param new_thread The newly created thread handle.
 * @param func The function to start the new thread in
 * @param userData Any data to be passed to the starting function
 */
awe_status_t awe_thread_create(awe_thread_t** thread, awe_thread_process process,
								void *userdata);

void awe_thread_destroy(awe_thread_t *thread);

/*
 * Start the thread in awe_thread_process.
 */
awe_status_t awe_thread_start(awe_thread_t *thread, const char *thread_name, int32_t priority,
        					size_t stack);

/**
 * Get the thread ID
 */
os_thread_t awe_thread_id(awe_thread_t *thread);

/**
 * awe_thread_exit_pending() returns true if awe_thread_request_exit() has been called.
 */
int awe_thread_exit_pending(awe_thread_t *thread);

/**
 * Indicates whether this thread is running or not.
 */
int awe_thread_running(awe_thread_t *thread);

/**
 * Ask this object's thread to exit. This function is asynchronous, when the
 * function returns the thread might still be running. Of course, this
 * function can be called from a different thread.
 */
awe_status_t awe_thread_request_exit(awe_thread_t *thread);

//------------------------

/**
 * Get the thread ID
 */
AWE_DECLARE(os_thread_t) awe_os_thread_id(void);

AWE_END_DECLS

#endif /* AWE_THREAD_H_ */
