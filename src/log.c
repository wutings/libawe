/*
 * log.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#if defined(_WIN32)
//
#else
#include <sys/time.h>
#include <pthread.h>
#endif

#define AWE_LOG_TAG "log"
#include "awe/log.h"
#include "awe/awe.h"

#define LOG_BUF_SIZE 1024

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int awe_log_level = ANDROID_LOG_DEFAULT;
//int awe_log_colors = 1;

static FILE* _log_file = NULL;

static awe_log_callback* _log_cb = NULL;

/*! \brief Coloured prefixes for errors and warnings logging. */
static const char *awe_log_prefix[] = {
/* no colors */
	"",
	"",
	"",
	"",
	"",
	"[WARN] ",
	"[ERR] ",
	"[FATAL] ",
/* with colors */
	"",
	"",
	"",
	"",
	"",
	ANSI_COLOR_YELLOW"[WARN]"ANSI_COLOR_RESET" ",
	ANSI_COLOR_RED"[ERR]"ANSI_COLOR_RESET" ",
	ANSI_COLOR_MAGENTA"[FATAL]"ANSI_COLOR_RESET" ",
};

AWE_DECLARE(void) __awe_log_assert(const char* file, const char* func, int line, const char* strexpr){
	char buf[LOG_BUF_SIZE];
	snprintf(buf, LOG_BUF_SIZE, "\n#\n# Fatal error in %s, %s, line:%d\n# Assertion failed:'%s'\n#\n#",
				file, func, line, strexpr);

	ALOG(LOG_FATAL, "", "%s", buf);

    abort(); /* abort so we have a chance to debug the situation */
}

AWE_DECLARE(void) __awe_log_fatal(const char* file, const char* func, int line, const char* strexpr, long int v1, long int v2){
	char buf[LOG_BUF_SIZE];

	snprintf(buf, LOG_BUF_SIZE, "\n#\n# Fatal error in %s, %s, line:%d\n# Check failed:'%s'=>(%ld vs %ld)\n#\n#",
			file, func, line, strexpr, v1, v2);

	ALOG(LOG_FATAL, "", "%s", buf);

	abort(); /* abort so we have a chance to debug the situation */
}

AWE_DECLARE(void) __awe_log_write(int prio, const char *tag, const char *fmt, ...){
	if(prio < awe_log_level) {
		return;
	}
	char buf[LOG_BUF_SIZE] = {0};
	char log_ts[64] = {0};

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
	va_end(ap);

	time_exp_t tp;
	awe_time_exp_lt(&tp, awe_time_us());
	snprintf(log_ts, sizeof(log_ts), "[%02d %02d %02d:%02d:%02d.%03d %lu]",
			tp.tm_mon + 1, tp.tm_mday, tp.tm_hour, tp.tm_min, tp.tm_sec, (tp.tm_usec / 1000),
			(unsigned long)awe_os_thread_id());

#if defined(ANDROID)
	__android_log_print(prio, tag, "%s", buf);
#else
	if(_log_cb == NULL){
		printf("%s%s[%s]%s\n", awe_log_prefix[prio], log_ts, tag, buf);
	}else{
		_log_cb(prio, log_ts, tag, buf);
	}
#endif

	if(_log_file != NULL){
		fprintf(_log_file, "%s%s[%s]%s\n", awe_log_prefix[prio], log_ts, tag, buf);
		fflush(_log_file);
	}
}

void awe_log_init(int prio, const char* filename, awe_log_callback* cb){
	if(prio > 0){
		awe_log_level = prio;
	}
	if(filename != NULL && strlen(filename) > 0){
		if(_log_file == NULL){
			_log_file = fopen(filename, "a");
		}
	}else{
		_log_file = NULL;
	}

	_log_cb = cb;
}

void awe_log_deinit(void){
	if(_log_file != NULL){
		fclose(_log_file);
		_log_file = NULL;
	}
}
