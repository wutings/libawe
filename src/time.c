/*
 * time.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

/* System Headers required for time library */
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#include "awe/time.h"
#include "config.h"

/* Define to 1 if `tm_gmtoff' is a member of `struct tm'. */
#define HAVE_STRUCT_TM_TM_GMTOFF 1

int64_t awe_time_ms(void){
	return awe_time_us() / 1000;
}

int64_t awe_time_us(void){
#ifndef __APPLE__
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (int64_t)ts.tv_sec * AWE_USEC_PER_SEC + ts.tv_nsec / 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * AWE_USEC_PER_SEC + tv.tv_usec;
#endif
}

#if !defined(HAVE_STRUCT_TM_TM_GMTOFF) && !defined(HAVE_STRUCT_TM___TM_GMTOFF)
static int32_t server_gmt_offset;
#define NO_GMTOFF_IN_STRUCT_TM
#endif

static long get_offset(struct tm *tm){
#if defined(HAVE_STRUCT_TM_TM_GMTOFF)
    return tm->tm_gmtoff;
#elif defined(HAVE_STRUCT_TM___TM_GMTOFF)
    return tm->__tm_gmtoff;
#else
#ifdef NETWARE
    /* Need to adjust the global variable each time otherwise
        the web server would have to be restarted when daylight
        savings changes.
    */
    if (daylightOnOff) {
        return server_gmt_offset + daylightOffset;
    }
#else
    if (tm->tm_isdst)
        return server_gmt_offset + 3600;
#endif
    return server_gmt_offset;
#endif
}

static void explode_time(time_exp_t *xt, int64_t t,
                         int32_t offset, int use_localtime){
    struct tm tm;
    time_t tt = (t / AWE_USEC_PER_SEC) + offset;
    xt->tm_usec = t % AWE_USEC_PER_SEC;

#if defined (_POSIX_THREAD_SAFE_FUNCTIONS)
    if (use_localtime)
        localtime_r(&tt, &tm);
    else
        gmtime_r(&tt, &tm);
#else
    if (use_localtime)
        tm = *localtime(&tt);
    else
        tm = *gmtime(&tt);
#endif

    xt->tm_sec  = tm.tm_sec;
    xt->tm_min  = tm.tm_min;
    xt->tm_hour = tm.tm_hour;
    xt->tm_mday = tm.tm_mday;
    xt->tm_mon  = tm.tm_mon;
    xt->tm_year = tm.tm_year;
    xt->tm_wday = tm.tm_wday;
    xt->tm_yday = tm.tm_yday;
    xt->tm_isdst = tm.tm_isdst;
    xt->tm_gmtoff = (int32_t)get_offset(&tm);
}

int awe_time_exp_lt(time_exp_t *result, int64_t input){
    explode_time(result, input, 0, 1);
    result->tm_gmtoff = 0;
    return 0;
}

int awe_sleep(unsigned ms){
	struct timeval tv;
	tv.tv_sec = ms / 1000;
	tv.tv_usec = (ms % 1000) * 1000;
	select(0, NULL, NULL, NULL, &tv);
	return 0;
}
