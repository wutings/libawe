/*
 * time.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_TIME_H_
#define AWE_TIME_H_

#include <stdint.h>
#include <string.h>

#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <awe/awe.h>

/** number of microseconds per second */
#define AWE_USEC_PER_SEC INT64_C(1000000)

/** @return apr_time_t as a second */
#define awe_time_sec(time) ((time) / AWE_USEC_PER_SEC)

AWE_BEGIN_DECLS

/**
 * a structure similar to ANSI struct tm with the following differences:
 *  - tm_usec isn't an ANSI field
 *  - tm_gmtoff isn't an ANSI field (it's a BSDism)
 */
typedef struct time_exp_t {
    /** microseconds past tm_sec */
    int32_t tm_usec;
    /** (0-61) seconds past tm_min */
    int32_t tm_sec;
    /** (0-59) minutes past tm_hour */
    int32_t tm_min;
    /** (0-23) hours past midnight */
    int32_t tm_hour;
    /** (1-31) day of the month */
    int32_t tm_mday;
    /** (0-11) month of the year */
    int32_t tm_mon;
    /** year since 1900 */
    int32_t tm_year;
    /** (0-6) days since Sunday */
    int32_t tm_wday;
    /** (0-365) days since January 1 */
    int32_t tm_yday;
    /** daylight saving time */
    int32_t tm_isdst;
    /** seconds east of UTC */
    int32_t tm_gmtoff;
}time_exp_t;

/**
 * Get the current time in milliseconds.
 * @return milliseconds.
 */
int64_t awe_time_ms(void);

/**
 * Get the current time in microseconds.
 * @return microseconds.
 */
int64_t awe_time_us(void);

/**
 * Sleep for a given time.  Although the duration is expressed in
 * microseconds, the actual delay may be rounded to the precision of the
 * system timer.
 *
 * @param  ms Number of microseconds to sleep.
 * @return zero on success or (negative) error code.
 */
int awe_sleep(uint32_t ms);
/**
 * Convert a time to its human readable components (GMT).
 * @param result the exploded time
 * @param input the time to explode
 */
int awe_time_exp_lt(time_exp_t *result, int64_t input);

#if defined(_WIN32)

struct timezone {
	int tz_minuteswest; /* minutes W of Greenwich */
	int tz_dsttime; /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz);

#endif

AWE_END_DECLS

#endif /* AWE_TIME_H_ */
