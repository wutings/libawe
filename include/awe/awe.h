/*
 * awe.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_AWE_H_
#define AWE_AWE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

/**
 * Type for specifying an error or status code.
 */
typedef int awe_status_t;

/** FALSE */
#ifndef FALSE
#define FALSE 0
#endif
/** TRUE */
#ifndef TRUE
#define TRUE (!FALSE)
#endif

typedef bool awe_bool;

/**
 * AWE public API wrap for C++ compilers.
 */
#ifdef __cplusplus
#define AWE_BEGIN_DECLS     extern "C" {
#define AWE_END_DECLS       }
#else
#define AWE_BEGIN_DECLS
#define AWE_END_DECLS
#endif

#if defined(DOXYGEN) || !defined(_WIN32)
#define AWE_DECLARE(type)            type
#define AWE_DECLARE_NONSTD(type)     type
#define AWE_DECLARE_DATA
#elif defined(AWE_DECLARE_STATIC)
#define AWE_DECLARE(type)            type __stdcall
#define AWE_DECLARE_NONSTD(type)     type __cdecl
#define AWE_DECLARE_DATA
#elif defined(AWE_DECLARE_EXPORT)
#define AWE_DECLARE(type)            __declspec(dllexport) type __stdcall
#define AWE_DECLARE_NONSTD(type)     __declspec(dllexport) type __cdecl
#define AWE_DECLARE_DATA             __declspec(dllexport)
#else
#define AWE_DECLARE(type)            __declspec(dllimport) type __stdcall
#define AWE_DECLARE_NONSTD(type)     __declspec(dllimport) type __cdecl
#define AWE_DECLARE_DATA             __declspec(dllimport)
#endif

#define MODULE_STATUS_RESET(var, s)	var = (s)
#define MODULE_STATUS_SET(var, s)	((var) |= (s))
#define MODULE_STATUS_CLR(var, s)	((var) &= (~(s)))
#define MODULE_STATUS_ISSET(var, s)	((var) & (s))

#include <awe/errors.h>
#include <awe/atomic.h>
#include <awe/object.h>
#include <awe/mem.h>
#include <awe/dlist.h>
#include <awe/thread.h>
#include <awe/mutex.h>
#include <awe/cond.h>
#include <awe/rwlock.h>
#include <awe/timer_task.h>
#include <awe/socket_helper.h>
#include <awe/socketpair.h>
#include <awe/linkedlist.h>
#include <awe/ringbuffer.h>
#include <awe/ringarray.h>
#include <awe/buffer.h>
#include <awe/packet.h>
#include <awe/looper.h>

#endif /* AWE_AWE_H_ */
