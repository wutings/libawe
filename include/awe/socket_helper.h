/*
 * socket_helper.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_SOCKET_HELPER_H_
#define AWE_SOCKET_HELPER_H_

#if defined(_WIN32)
#define _WIN32_WINNT 0x0501     // Windows XP Support


/* Windows */
//#if defined(WINNT) || defined(_WINNT) || defined(__BORLANDC__) || defined(__MINGW32__)
//	|| defined(_WIN32_WCE) || defined (_MSC_VER)
#define _MSWSOCK_
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#include <process.h>
//#endif
#include <windows.h>
//#include <errno.h>
#include <string.h>

#define socket_close	closesocket
#define socklen_t		int

#ifdef EWOULDBLOCK
#undef EWOULDBLOCK
#endif
#ifdef EINPROGRESS
#undef EINPROGRESS
#endif
#ifdef EAGAIN
#undef EAGAIN
#endif
#ifdef EINTR
#undef EINTR
#endif

#define EWOULDBLOCK	WSAEWOULDBLOCK
#define EINPROGRESS	WSAEWOULDBLOCK
#define EAGAIN		WSAEWOULDBLOCK
#define EINTR		WSAEINTR

#if defined(_WIN32_WCE)
#define NO_STRSTREAM 1
#endif

//// For "uintptr_t" and "intptr_t", we assume that if they're not already defined, then this must be
//// an old, 32-bit version of Windows:
//#if !defined(_MSC_STDINT_H_) && !defined(_UINTPTR_T_DEFINED) && !defined(_UINTPTR_T_DECLARED) && !defined(_UINTPTR_T)
//typedef unsigned uintptr_t;
//#endif
//#if !defined(_MSC_STDINT_H_) && !defined(_INTPTR_T_DEFINED) && !defined(_INTPTR_T_DECLARED) && !defined(_INTPTR_T)
//typedef int intptr_t;
//#endif

#elif defined(VXWORKS)
/* VxWorks */
#include <time.h>
#include <timers.h>
#include <sys/times.h>
#include <sockLib.h>
#include <hostLib.h>
#include <resolvLib.h>
#include <ioLib.h>
#else
/* Unix */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdbool.h>
#if defined(_QNX4)
#include <sys/select.h>
#include <unix.h>
#endif

#define socket_close close

#endif

//#ifndef SOCKLEN_T
//#define SOCKLEN_T int
//#endif

#include <awe/awe.h>

#define D_NO_SOCKADDR_LEN

#ifdef D_NO_SOCKADDR_LEN
#define SET_SOCKADDR_SIN_LEN(var)
#else
#define SET_SOCKADDR_SIN_LEN(var) var.sin_len = sizeof(var)
#endif


#if defined(_WIN32)
#define S_ERRNO WSAGetLastError()

#if _WIN32_WINNT < 0x0600
#undef inet_ntop
const char* inet_ntop(int af, const void *src, char *dst, socklen_t size);
#undef inet_pton
int inet_pton(int af, const char *src, void *dst);
#endif
#else
#define S_ERRNO errno
#endif

AWE_BEGIN_DECLS

typedef struct socket_addr{
	int 		_af; //AF_INET AF_INET6
	socklen_t	_addrlen;
	char		_addr[INET6_ADDRSTRLEN];//struct sockaddr*
}socket_addr;

AWE_DECLARE(int) awe_socket_addrinfo(socket_addr *sa, const char *host, uint16_t port);
AWE_DECLARE(awe_status_t) awe_socket_setnodelay(int socketNum);

AWE_DECLARE(void) awe_socket_ignore_sigpipe(int socketNum);

AWE_DECLARE(int) awe_socket_make_nonblocking(int socketNum);
AWE_DECLARE(int) awe_socket_make_blocking(int socketNum, unsigned write_timeout_ms);

AWE_DECLARE(int) awe_socket_create(int domain, int type);

AWE_DECLARE(int) awe_socket_stream_setup(int domain, uint16_t port, bool make_nonBlocking);
AWE_DECLARE(int) awe_socket_datagram_setup(int domain, uint16_t port, bool make_nonBlocking);

AWE_DECLARE(int) awe_socket_read(int socketNum, void* buffer, size_t size);
AWE_DECLARE(int) awe_socket_write(int socketNum, const void* buffer, size_t size);

AWE_DECLARE(int) awe_socket_sendto(int socketNum, const void* buffer, size_t bufferSize,
		const struct sockaddr* addr, socklen_t addrlen);
AWE_DECLARE(int) awe_socket_recvfrom(int socketNum, void* buffer, size_t bufferSize,
		struct sockaddr *fromAddress, socklen_t *addrlen);

AWE_DECLARE(unsigned) awe_socket_getSendBufferSize(int socketNum);
AWE_DECLARE(unsigned) awe_socket_getReceiveBufferSize(int socketNum);

AWE_DECLARE(unsigned) awe_socket_setSendBufferTo(int socketNum, unsigned requestedSize);
AWE_DECLARE(unsigned) awe_socket_setReceiveBufferTo(int socketNum, unsigned requestedSize);

AWE_DECLARE(awe_status_t) awe_socket_localHost4(int socketNum, char *buf, int buflen);
AWE_DECLARE(awe_status_t) awe_socket_localHost6(int socketNum, char *buf, int buflen);
#define awe_socket_localHost(af, s, b, l) \
	((af) == AF_INET) ? awe_socket_localHost4(s, b, l) : awe_socket_localHost6(s, b, l)

AWE_DECLARE(awe_status_t) awe_socket_localPort4(int socketNum, uint16_t *port);
AWE_DECLARE(awe_status_t) awe_socket_localPort6(int socketNum, uint16_t *port);
#define awe_socket_localPort(af, s, p) \
	((af) == AF_INET) ? awe_socket_localPort4(s, p) : awe_socket_localPort6(s, p)


AWE_DECLARE(awe_status_t) awe_socket_inet_ntop4(struct sockaddr* in, char* buf, int buflen, uint16_t* port);
AWE_DECLARE(awe_status_t) awe_socket_inet_ntop6(struct sockaddr* in, char* buf, int buflen, uint16_t* port);
#define awe_socket_inet_ntop(af, in, b, l, p) \
	((af) == AF_INET) ? awe_socket_inet_ntop4(in, b, l, p) : awe_socket_inet_ntop6(in, b, l, p)


AWE_DECLARE(int) awe_socket_connected(int socketNum, int* errcode);

//AWE_DECLARE(awe_status_t) awe_socket_connecttimeout(int socketNum, int32_t timeoutMs);
AWE_DECLARE(awe_status_t) awe_socket_wait_for_connect(int socketNum, int32_t timeoutMs, int* errcode);

AWE_DECLARE(int) awe_socket_readable(int socketNum, int32_t timeoutMs);
AWE_DECLARE(int) awe_socket_writable(int socketNum, int32_t timeoutMs);

AWE_DECLARE(int) awe_socket_setdscp(int socketNum, int iptos);

AWE_END_DECLS

#endif /* AWE_SOCKET_HELPER_H_ */

