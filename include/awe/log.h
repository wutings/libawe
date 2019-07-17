/*
 * log.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_LOG_H_
#define AWE_LOG_H_

#include <stdarg.h>

#include <awe/awe.h>

#if defined(ANDROID)
#include <android/log.h>
#else
/*
 * Android log priority values, in ascending priority order.
 */
typedef enum android_LogPriority {
    ANDROID_LOG_UNKNOWN = 0,
    ANDROID_LOG_DEFAULT,    /* only for SetMinPriority() */
    ANDROID_LOG_VERBOSE,
    ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,
    ANDROID_LOG_WARN,
    ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL,
//    ANDROID_LOG_SILENT,     /* only for SetMinPriority(); must be last */
} android_LogPriority;

//extern int sw_log_level;
//extern int sw_log_colors;
//
//extern const char *sw_log_prefix[];

#endif //ANDROID end

// ---------------------------------------------------------------------

/*
 * This is the local tag used for the following simplified
 * logging macros.  You can change this preprocessor definition
 * before using the other macros to change the tag.
 */
#ifndef AWE_LOG_TAG
#error "No AWE_LOG_TAG"
#endif

// ---------------------------------------------------------------------

#ifndef __predict_false
#if defined(__clang__) || defined(__GNUC__)
#define __predict_false(exp)	__builtin_expect((exp) != 0, 0)
#else
#define __predict_false(exp)	(!!(exp))
#endif
#endif

/*
 * Simplified macro to send a verbose log message using the current AWE_LOG_TAG.
 */
#ifndef ALOGV
#define ALOGV(...) ALOG(LOG_VERBOSE, AWE_LOG_TAG, __VA_ARGS__)
#endif

#ifndef ALOGV_IF
#define ALOGV_IF(cond, ...) \
		do{\
			if(__predict_false(cond)){\
				ALOG(LOG_VERBOSE, AWE_LOG_TAG, __VA_ARGS__); \
			}\
		}while(0)
#endif

/*
 * Simplified macro to send a debug log message using the current AWE_LOG_TAG.
 */
#ifndef ALOGD
#define ALOGD(...) ALOG(LOG_DEBUG, AWE_LOG_TAG, __VA_ARGS__)
#endif

#ifndef ALOGD_IF
#define ALOGD_IF(cond, ...) \
		do{\
			if(__predict_false(cond)){\
				ALOG(LOG_DEBUG, AWE_LOG_TAG, __VA_ARGS__); \
			}\
		}while(0)
#endif

/*
 * Simplified macro to send an info log message using the current AWE_LOG_TAG.
 */
#ifndef ALOGI
#define ALOGI(...) ALOG(LOG_INFO, AWE_LOG_TAG, __VA_ARGS__)
#endif

#ifndef ALOGI_IF
#define ALOGI_IF(cond, ...) \
		do{\
			if(__predict_false(cond)){\
				ALOG(LOG_INFO, AWE_LOG_TAG, __VA_ARGS__); \
			}\
		}while(0)
#endif

/*
 * Simplified macro to send a warning log message using the current AWE_LOG_TAG.
 */
#ifndef ALOGW
#define ALOGW(...) ALOG(LOG_WARN, AWE_LOG_TAG, __VA_ARGS__)
#endif

#ifndef ALOGW_IF
#define ALOGW_IF(cond, ...) \
		do{\
			if(__predict_false(cond)){\
				ALOG(LOG_WARN, AWE_LOG_TAG, __VA_ARGS__); \
			}\
		}while(0)
#endif

/*
 * Simplified macro to send an error log message using the current AWE_LOG_TAG.
 */
#ifndef ALOGE
#define ALOGE(...) ALOG(LOG_ERROR, AWE_LOG_TAG, __VA_ARGS__)
#endif

#ifndef ALOGE_IF
#define ALOGE_IF(cond, ...) \
		do{\
			if(__predict_false(cond)){\
				ALOG(LOG_ERROR, AWE_LOG_TAG, __VA_ARGS__); \
			}\
		}while(0)
#endif

/*
 * Simplified macro to send an error log message using the current AWE_LOG_TAG.
 */
#ifndef ALOGF
#define ALOGF(...) ALOG(LOG_FATAL, AWE_LOG_TAG, __VA_ARGS__)
#endif

#ifndef ALOGF_IF
#define ALOGF_IF(cond, ...) \
		do{\
			if(__predict_false(cond)){\
				ALOG(LOG_FATAL, AWE_LOG_TAG, __VA_ARGS__); \
			}\
		}while(0)
#endif

// ---------------------------------------------------------------------

/*
 * Basic log message macro.
 *
 * Example:
 *  ALOG(LOG_WARN, NULL, "Failed with error %d", errno);
 *
 * The second argument may be NULL or "" to indicate the "global" tag.
 */
#ifndef ALOG
#define ALOG(priority, tag, ...) \
    LOG_PRI(ANDROID_##priority, tag, __VA_ARGS__)
#endif

/*
 * Log macro that allows you to specify a number for the priority.
 */
#ifndef LOG_PRI
#define LOG_PRI(priority, tag, ...) \
    __awe_printLog(priority, tag, __VA_ARGS__)
#endif

// ===========================================================================
/*
 * The stuff in the rest of this file should not be used directly.
 */
#ifndef __awe_printLog
#define __awe_printLog(prio, tag, ...) __awe_log_write(prio, tag, ##__VA_ARGS__)
#endif

// ---------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

typedef void awe_log_callback(int prio, const char* log_ts, const char* tag, const char* log);

void awe_log_init(int prio, const char* filename, awe_log_callback* cb);
void awe_log_deinit(void);

AWE_DECLARE(void) __awe_log_write(int prio, const char *tag, const char *fmt, ...);
AWE_DECLARE(void) __awe_log_assert(const char* file, const char* func, int line, const char* strexpr);
AWE_DECLARE(void) __awe_log_fatal(const char* file, const char* func, int line,
		const char* strexpr, long int v1, long int v2);

#ifdef __cplusplus
}
#endif

#define DEFINE_AWE_CHECK_OP_IMPL(name, op)                                   \
  static inline void __Check##name##Impl(long int v1, long int v2, const char* __strexpr, \
	const char* file, const char* func, int line) { 						\
    if (__predict_false(!(v1 op v2)))                                       \
    	__awe_log_fatal(file, func, line, __strexpr, v1, v2);         		\
  }

DEFINE_AWE_CHECK_OP_IMPL(EQ, ==)
DEFINE_AWE_CHECK_OP_IMPL(NE, !=)
DEFINE_AWE_CHECK_OP_IMPL(LE, <=)
DEFINE_AWE_CHECK_OP_IMPL(LT, < )
DEFINE_AWE_CHECK_OP_IMPL(GE, >=)
DEFINE_AWE_CHECK_OP_IMPL(GT, > )

// Helper macro for binary operators.
// Don't use this macro directly in your code, use AWE_CHECK_EQ et al below.
//
#define AWE_CHECK_OP(name, op, val1, val2) \
		__Check##name##Impl((val1), (val2), #val1 " " #op " " #val2, __FILE__, __func__, __LINE__)

#define AWE_CHECK_EQ(val1, val2) AWE_CHECK_OP(EQ, ==, val1, val2)
#define AWE_CHECK_NE(val1, val2) AWE_CHECK_OP(NE, !=, val1, val2)
#define AWE_CHECK_LE(val1, val2) AWE_CHECK_OP(LE, <=, val1, val2)
#define AWE_CHECK_LT(val1, val2) AWE_CHECK_OP(LT, < , val1, val2)
#define AWE_CHECK_GE(val1, val2) AWE_CHECK_OP(GE, >=, val1, val2)
#define AWE_CHECK_GT(val1, val2) AWE_CHECK_OP(GT, > , val1, val2)

// ---------------------------------------------------------------------

//assert true
#ifndef ALOG_ASSERT
#define ALOG_ASSERT(cond) \
    ( (__predict_false(cond)) \
    ? ((void)__awe_log_assert(__FILE__, __func__, __LINE__, #cond)) \
    : (void)0 )
#endif

#endif /* AWE_LOG_H_ */
