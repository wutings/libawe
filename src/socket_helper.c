/*
 * socket_helper.c
 *
 */
#if defined(_WIN32) || defined(_QNX4)
#else
#include <signal.h>
#define USE_SIGNALS 1
#endif

#define AWE_LOG_TAG "socket_helper"
#include "awe/log.h"
#include "awe/errors.h"
#include "awe/socket_helper.h"

//#ifndef MAKE_SOCKADDR_IN
/*adr,prt must be in network order*/
#define MAKE_SOCKADDR_IN(var, adr, prt)	\
    struct sockaddr_in var;				\
    bzero(&var, sizeof(struct sockaddr_in));\
    var.sin_family = AF_INET;			\
    var.sin_port = (prt);				\
    var.sin_addr.s_addr = (adr);		\
    SET_SOCKADDR_SIN_LEN(var)

#define MAKE_SOCKADDR_IN6(var, adr, prt)	\
	struct sockaddr_in6 var;				\
	bzero(&var, sizeof(struct sockaddr_in6));\
	var.sin6_family = AF_INET6;			\
	var.sin6_port = (prt);				\
	var.sin6_addr = (adr);
//#endif

AWE_DECLARE(int) awe_socket_addrinfo(socket_addr *sa, const char *host, uint16_t port){
	struct addrinfo hints;
	struct addrinfo* answer = NULL;
	struct addrinfo* curr = NULL;
	do{
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_flags = 0;
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		char pp[8] = {0};
		snprintf(pp, sizeof(pp), "%d", port);

		int ret = 0;
		if((ret = getaddrinfo(host, pp, &hints, &answer)) != 0){
			ALOGE("socket_addrinfo, S_ERRNO:%d, %s(%s)", S_ERRNO, host, gai_strerror(ret));
			break;
		}

		for (curr = answer; curr != NULL; curr = curr->ai_next) {
			switch (curr->ai_family){
			case AF_INET:
			case AF_INET6:{
				sa->_af = curr->ai_family;
				sa->_addrlen = curr->ai_addrlen;
				memcpy(sa->_addr, curr->ai_addr, curr->ai_addrlen);
				break;
			}
			}
		}
		freeaddrinfo(answer);
		return 0;
	}while(0);
	freeaddrinfo(answer);
	return -1;
}

AWE_DECLARE(awe_status_t) awe_socket_setnodelay(int socketNum){
	int flag = 1;
	setsockopt(socketNum, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
    return AWE_OK;
}

AWE_DECLARE(void) awe_socket_ignore_sigpipe(int socketNum) {
#ifdef USE_SIGNALS
#ifdef SO_NOSIGPIPE
	int set_option = 1;
	setsockopt(socketNum, SOL_SOCKET, SO_NOSIGPIPE, &set_option, sizeof(set_option));
#else
	signal(SIGPIPE, SIG_IGN);
#endif
#endif
}

AWE_DECLARE(int) awe_socket_make_nonblocking(int socketNum) {
#if defined(_WIN32)
	unsigned long arg = 1;
	return ioctlsocket(socketNum, FIONBIO, &arg) == 0;
#elif defined(VXWORKS)
	int arg = 1;
	return ioctl(socketNum, FIONBIO, (int)&arg) == 0;
#else
	int curFlags = fcntl(socketNum, F_GETFL, 0);
	return fcntl(socketNum, F_SETFL, curFlags | O_NONBLOCK) >= 0;
#endif
}

AWE_DECLARE(int) awe_socket_make_blocking(int socketNum, unsigned write_timeout_ms) {
	int result;
#if defined(_WIN32)
	unsigned long arg = 0;
	result = ioctlsocket(socketNum, FIONBIO, &arg) == 0;
#elif defined(VXWORKS)
	int arg = 0;
	result = ioctl(socketNum, FIONBIO, (int)&arg) == 0;
#else
	int curFlags = fcntl(socketNum, F_GETFL, 0);
	result = fcntl(socketNum, F_SETFL, curFlags & (~O_NONBLOCK)) >= 0;
#endif

	if (write_timeout_ms > 0) {
#ifdef SO_SNDTIMEO
		struct timeval tv;
		tv.tv_sec = write_timeout_ms / 1000;
		tv.tv_usec = (write_timeout_ms % 1000) * 1000;
		setsockopt(socketNum, SOL_SOCKET, SO_SNDTIMEO, (char *) &tv, sizeof(tv));
#endif
	}

	return result;
}

AWE_DECLARE(int) awe_socket_create(int domain, int type) {
	// Call "socket()" to create a (IPv4 | IPv6) socket of the specified type.
	// But also set it to have the 'close on exec' property (if we can)
	int fd;

#ifdef SOCK_CLOEXEC
	fd = socket(domain, type | SOCK_CLOEXEC, 0);
	if (fd != -1 || S_ERRNO != EINVAL) return fd;
	// An "S_ERRNO" of EINVAL likely means that the system wasn't happy with the SOCK_CLOEXEC;
	// fall through and try again without it:
#endif

	fd = socket(domain, type, 0);
#ifdef FD_CLOEXEC
	if (fd != -1) fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
	return fd;
}

AWE_DECLARE(int) awe_socket_stream_setup(int domain, uint16_t port, bool make_nonBlocking) {
	int new_socket = awe_socket_create(domain, SOCK_STREAM);
	if (new_socket < 0) {
		ALOGE("unable to create stream socket, errno:%d, %s.", S_ERRNO, strerror(S_ERRNO));
		return new_socket;
	}

	int reuseFlag = 1;
	if (setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR,
		 (const char*)&reuseFlag, sizeof(reuseFlag)) < 0) {
		ALOGE("setsockopt(SO_REUSEADDR) error:%d.", S_ERRNO);
		socket_close(new_socket);
		return -1;
	}

	// SO_REUSEPORT doesn't really make sense for TCP sockets, so we
	// normally don't set them.  However, if you really want to do this
	// #define REUSE_FOR_TCP
#ifdef REUSE_FOR_TCP
#if defined(_WIN32)
  // Windoze doesn't properly handle SO_REUSEPORT
#else
#ifdef SO_REUSEPORT
  if (setsockopt(new_socket, SOL_SOCKET, SO_REUSEPORT,
		 (const char*)&reuseFlag, sizeof(reuseFlag)) < 0) {
	  ALOGE("setsockopt(SO_REUSEPORT) error:%d.", S_ERRNO);
	  socket_close(new_socket);
	  return -1;
  }
#endif
#endif
#endif

	// Note: Windoze requires binding, even if the port number is 0
#if defined(_WIN32)
#else
	if (port != 0/* || ReceivingInterfaceAddr != INADDR_ANY*/) {
#endif
		int bindrs = 0;
		if(domain == AF_INET){
			MAKE_SOCKADDR_IN(name, INADDR_ANY, htons(port));
			bindrs = bind(new_socket, (struct sockaddr*) &name, sizeof(name));
		}else{
			MAKE_SOCKADDR_IN6(name, in6addr_any, htons(port));
			bindrs = bind(new_socket, (struct sockaddr*) &name, sizeof(name));
		}

		if (bindrs != 0) {
			ALOGE("bind() error (port number:%d):%d.", port, S_ERRNO);
			socket_close(new_socket);
			return -1;
		}
#if defined(_WIN32)
#else
	}
#endif

	if (make_nonBlocking) {
		if (!awe_socket_make_nonblocking(new_socket)) {
			ALOGE("failed to make non-blocking:%d.", S_ERRNO);
			socket_close(new_socket);
			return -1;
		}
	}

	return new_socket;
}

AWE_DECLARE(int) awe_socket_datagram_setup(int domain, uint16_t port, bool make_nonBlocking) {
	int new_socket = awe_socket_create(domain, SOCK_DGRAM);
	if (new_socket < 0) {
		ALOGE("unable to create datagram socket, errno:%d, %s.", S_ERRNO, strerror(S_ERRNO));
		return new_socket;
	}

	int reuseFlag = 1;
	if (setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR,
			(const char*) &reuseFlag, sizeof(reuseFlag)) < 0) {
		ALOGE("setsockopt(SO_REUSEADDR) error:%d.", S_ERRNO);
		socket_close(new_socket);
		return -1;
	}

#if defined(_WIN32)
	// Windoze doesn't properly handle SO_REUSEPORT or IP_MULTICAST_LOOP
#else
//#ifdef SO_REUSEPORT
//	if (setsockopt(new_socket, SOL_SOCKET, SO_REUSEPORT,
//					(const char*)&reuseFlag, sizeof(reuseFlag)) < 0) {
//		ALOGE("setsockopt(SO_REUSEPORT) error:%d.", S_ERRNO);
//		socket_close(new_socket);
//		return -1;
//	}
//#endif

#ifdef IP_MULTICAST_LOOP
	const u_int8_t loop = 1;
	if (setsockopt(new_socket, IPPROTO_IP, IP_MULTICAST_LOOP,
			(const char*) &loop, sizeof(loop)) < 0) {
		ALOGE("setsockopt(IP_MULTICAST_LOOP) error:%d.", S_ERRNO);
		socket_close(new_socket);
		return -1;
	}
#endif
#endif

	// Note: Windoze requires binding, even if the port number is 0
#if defined(_WIN32)
#else
	if (port != 0 /*|| ReceivingInterfaceAddr != INADDR_ANY*/) {
#endif
		int bindrs = 0;
		if(domain == AF_INET){
			MAKE_SOCKADDR_IN(name, INADDR_ANY, htons(port));
			bindrs = bind(new_socket, (struct sockaddr*) &name, sizeof(name));
		}else{
			MAKE_SOCKADDR_IN6(name, in6addr_any, htons(port));
			bindrs = bind(new_socket, (struct sockaddr*) &name, sizeof(name));
		}

		if (bindrs != 0) {
			ALOGE("bind() error (port number:%d):%d.", port, S_ERRNO);
			socket_close(new_socket);
			return -1;
		}
#if defined(_WIN32)
#else
	}
#endif

//	// Set the sending interface for multicasts, if it's not the default:
//	if (SendingInterfaceAddr != INADDR_ANY) {
//		struct in_addr addr;
//		addr.s_addr = SendingInterfaceAddr;
//
//		if (setsockopt(new_socket, IPPROTO_IP, IP_MULTICAST_IF,
//				(const char*) &addr, sizeof(addr)) < 0) {
////			socketErr(env, "error setting outgoing multicast interface:");
//			socket_close(new_socket);
//			return -1;
//		}
//	}

	if (make_nonBlocking) {
		if (!awe_socket_make_nonblocking(new_socket)) {
			ALOGE("failed to make non-blocking:%d.", S_ERRNO);
			socket_close(new_socket);
			return -1;
		}
	}

	return new_socket;
}


AWE_DECLARE(int) awe_socket_read(int socketNum, void* buffer, size_t size){
	int bytesRead = (int)recv(socketNum, buffer, size, 0);
	if(bytesRead > 0){
		return bytesRead;
	}else if(bytesRead < 0){
		int err = S_ERRNO;
		if(err == EINTR || err == EAGAIN){
			return 0;
		}
	}
	return -1;
}

AWE_DECLARE(int) awe_socket_write(int socketNum, const void* buffer, size_t size){
	int bytesWrite = (int)send(socketNum, buffer, size, 0);
	if(bytesWrite > 0){
		return bytesWrite;
	}else if(bytesWrite < 0){
		int err = S_ERRNO;
		if(err == EINTR || err == EAGAIN){
			return 0;
		}
	}
	return -1;
}

AWE_DECLARE(int) awe_socket_recvfrom(int socketNum, void* buffer, size_t bufferSize,
		struct sockaddr *fromAddress, socklen_t *addrlen) {
	int bytesRead = recvfrom(socketNum, buffer, bufferSize, 0, fromAddress, addrlen);
	if(bytesRead > 0){
		return bytesRead;
	}else if (bytesRead < 0) {
		//##### HACK to work around bugs in Linux and Windows:
		int err = S_ERRNO;
		if (err == EINTR || err == ECONNREFUSED
#if defined(_WIN32)
				// What a piece of crap Windows is.  Sometimes
				// recvfrom() returns -1, but with an 'errno' of 0.
				// This appears not to be a real error; just treat
				// it as if it were a read of zero bytes, and hope
				// we don't have to do anything else to 'reset'
				// this alleged error:
				|| err == 0 || err == EWOULDBLOCK
#else
				|| err == EAGAIN
#endif
				|| err == EHOSTUNREACH) { // Why does Linux return this for datagram socket?
			return 0;
		}
		//##### END HACK
    	ALOGE("recvfrom() error:%d.", S_ERRNO);
	} else if (bytesRead == 0) {
		// "recvfrom()" on a stream socket can return 0 if the remote end has closed the connection.
		//  Treat this as an error:
		return -1;
	}

	return bytesRead;
}

AWE_DECLARE(int) awe_socket_sendto(int socketNum, const void* buffer, size_t bufferSize,
		const struct sockaddr* addr, socklen_t addrlen) {
	int bytesSent = sendto(socketNum, buffer, bufferSize, 0, addr, addrlen);
	if(bytesSent > 0){
		return bytesSent;
	}else if (bytesSent < 0) {
		//##### HACK to work around bugs in Linux and Windows:
		int err = S_ERRNO;
		if (err == EINTR || err == ECONNREFUSED
#if defined(_WIN32)
				// What a piece of crap Windows is.  Sometimes
				// recvfrom() returns -1, but with an 'errno' of 0.
				// This appears not to be a real error; just treat
				// it as if it were a read of zero bytes, and hope
				// we don't have to do anything else to 'reset'
				// this alleged error:
				|| err == 0 || err == EWOULDBLOCK
#else
				|| err == EAGAIN
#endif
				|| err == EHOSTUNREACH) { // Why does Linux return this for datagram socket?
			return 0;
		}
		//##### END HACK
    	ALOGE("sendto() error:%d.", S_ERRNO);
	}
	return -1;
}

static unsigned socket_getBufferSize(int socketNum, int bufOptName) {
  unsigned curSize;
  socklen_t sizeSize = sizeof(curSize);
  if(getsockopt(socketNum, SOL_SOCKET, bufOptName, (char*)&curSize, &sizeSize) < 0) {
    return 0;
  }
  return curSize;
}

AWE_DECLARE(unsigned) awe_socket_getSendBufferSize(int socketNum) {
  return socket_getBufferSize(socketNum, SO_SNDBUF);
}

AWE_DECLARE(unsigned) awe_socket_getReceiveBufferSize(int socketNum) {
  return socket_getBufferSize(socketNum, SO_RCVBUF);
}

static unsigned awe_socket_setBufferTo(int socketNum, int bufOptName,
			    unsigned requestedSize) {
  socklen_t sizeSize = sizeof(requestedSize);
  setsockopt(socketNum, SOL_SOCKET, bufOptName, (char*)&requestedSize, sizeSize);

  // Get and return the actual, resulting buffer size:
  return socket_getBufferSize(socketNum, bufOptName);
}
AWE_DECLARE(unsigned) awe_socket_setSendBufferTo(int socketNum, unsigned requestedSize) {
	return awe_socket_setBufferTo(socketNum, SO_SNDBUF, requestedSize);
}
AWE_DECLARE(unsigned) awe_socket_setReceiveBufferTo(int socketNum, unsigned requestedSize) {
	return awe_socket_setBufferTo(socketNum, SO_RCVBUF, requestedSize);
}

AWE_DECLARE(awe_status_t) awe_socket_localHost4(int socketNum, char *buf, int buflen){
	struct sockaddr_in in4;
	socklen_t len = sizeof(struct sockaddr_in);
	if (getsockname(socketNum, (struct sockaddr*)&in4, &len) < 0){
		return -1;
	}
	return (inet_ntop(AF_INET, &in4.sin_addr, buf, buflen) != NULL) ? AWE_OK : -1;
}
AWE_DECLARE(awe_status_t) awe_socket_localHost6(int socketNum, char *buf, int buflen){
	struct sockaddr_in6 in6;
	socklen_t len = sizeof(struct sockaddr_in6);
	if (getsockname(socketNum, (struct sockaddr*)&in6, &len) < 0){
		return -1;
	}
	return (inet_ntop(AF_INET6, &in6.sin6_addr, buf, buflen) != NULL) ? AWE_OK : -1;
}

static awe_status_t socket_localPort4(int socketNum, uint16_t *resultPortNum/*host order*/) {
	struct sockaddr_in addr;
	addr.sin_port = 0;
	socklen_t len = sizeof(struct sockaddr_in);
	if (getsockname(socketNum, (struct sockaddr*) &addr, &len) < 0)
		return -1;

	*resultPortNum = ntohs(addr.sin_port);
	return AWE_OK;
}

AWE_DECLARE(awe_status_t) awe_socket_localPort4(int socketNum, uint16_t *port) {
	if (socket_localPort4(socketNum, port) != AWE_OK || *port == 0) {
		// Hack - call bind(), then try again:
		MAKE_SOCKADDR_IN(name, INADDR_ANY, 0);
		bind(socketNum, (struct sockaddr*)&name, sizeof(name));

		if (socket_localPort4(socketNum, port) != AWE_OK || *port == 0) {
			ALOGE("getsockname() error:%d.", S_ERRNO);
			return -1;
		}
	}
	return AWE_OK;
}

static awe_status_t socket_localPort6(int socketNum, uint16_t *resultPortNum/*host order*/) {
	struct sockaddr_in6 addr;
	addr.sin6_port = 0;
	socklen_t len = sizeof(struct sockaddr_in6);
	if (getsockname(socketNum, (struct sockaddr*) &addr, &len) < 0)
		return -1;

	*resultPortNum = ntohs(addr.sin6_port);
	return AWE_OK;
}

AWE_DECLARE(awe_status_t) awe_socket_localPort6(int socketNum, uint16_t *port){
	if (socket_localPort6(socketNum, port) != AWE_OK || *port == 0) {
		// Hack - call bind(), then try again:
		MAKE_SOCKADDR_IN6(name, in6addr_any, 0);
		bind(socketNum, (struct sockaddr*)&name, sizeof(name));

		if (socket_localPort6(socketNum, port) != AWE_OK || *port == 0) {
			ALOGE("getsockname() error:%d.", S_ERRNO);
			return -1;
		}
	}
	return AWE_OK;
}

AWE_DECLARE(awe_status_t) awe_socket_inet_ntop4(struct sockaddr* in, char* buf, int buflen, uint16_t* port){
	struct sockaddr_in* in4 = (struct sockaddr_in*)in;
	if(inet_ntop(AF_INET, &in4->sin_addr, buf, buflen) != NULL){
		*port = ntohs(in4->sin_port);
		return AWE_OK;
	}
	return -1;
}
AWE_DECLARE(awe_status_t) awe_socket_inet_ntop6(struct sockaddr* in, char* buf, int buflen, uint16_t* port){
	struct sockaddr_in6* in6 = (struct sockaddr_in6*)in;
	if(inet_ntop(AF_INET6, &in6->sin6_addr, buf, buflen) != NULL){
		*port = ntohs(in6->sin6_port);
		return AWE_OK;
	}
	return -1;
}

AWE_DECLARE(int) awe_socket_connected(int socketNum, int* errcode){
	int error = 0;
	socklen_t len = sizeof(error);
	// if connect success, it will return 0
	if (getsockopt(socketNum, SOL_SOCKET, SO_ERROR, (char*)&error, &len) < 0 || error != 0){
		*errcode = error;
		return -1;
	}
	return AWE_OK;
}

//awe_status_t awe_socket_connecttimeout(int socketNum, int32_t timeoutMs){
//	fd_set rset, wset;
//	FD_ZERO(&rset);
//	FD_SET(socketNum, &rset);
//	wset = rset;  // here block copy
//	struct timeval tv;
//	struct timeval *ptv = NULL;
//	if(timeoutMs >= 0){
//		tv.tv_sec = timeoutMs / 1000;
//		tv.tv_usec = (timeoutMs - (tv.tv_sec * 1000)) * 1000;
//		ptv = &tv;
//	}
//	int rs = select(socketNum+1, &rset, &wset, NULL, ptv);
//	if(rs == 0) {
//		return -EINPROGRESS;
//	}else if(rs == -1){
//		return -1;
//	}
//	if(FD_ISSET(socketNum, &rset) || FD_ISSET(socketNum, &wset)) {
//		if(awe_socket_connected(socketNum) != 0){
//			return -1;
//		}
//	}
//	return 0;
//}

AWE_DECLARE(awe_status_t) awe_socket_wait_for_connect(int socketNum, int32_t timeoutMs, int* errcode){
	fd_set wset, eset;
	FD_ZERO(&wset);
	FD_SET(socketNum, &wset);
	eset = wset;  // here block copy
	struct timeval tv;
	struct timeval *ptv = NULL;
	if(timeoutMs >= 0){
		tv.tv_sec = timeoutMs / 1000;
		tv.tv_usec = (timeoutMs - (tv.tv_sec * 1000)) * 1000;
		ptv = &tv;
	}
	int rs = select(socketNum+1, NULL, &wset, &eset, ptv);
	if(rs > 0) {
		if(FD_ISSET(socketNum, &wset) || FD_ISSET(socketNum, &eset)) {
			return awe_socket_connected(socketNum, errcode);
		}
	}else if(rs <= -1){
		*errcode = S_ERRNO;
		return -1;
	}
	return -AWE_CONN_EINPROGRESS;
}

AWE_DECLARE(int) awe_socket_readable(int socketNum, int32_t timeoutMs){
    struct timeval tv;
    struct timeval *ptv = NULL;
    fd_set readfds;

    if(socketNum <= 0){
		return -1;
	}

    if (timeoutMs >= 0) {
        // Calculate timeout value
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs - (tv.tv_sec * 1000)) * 1000;
        ptv = &tv;
    }

    FD_ZERO(&readfds);
    FD_SET(socketNum, &readfds);
    return select(socketNum + 1, &readfds, NULL, NULL, ptv);
}

AWE_DECLARE(int) awe_socket_writable(int socketNum, int32_t timeoutMs){
    struct timeval tv;
    struct timeval *ptv = NULL;
    fd_set writefds;

    if(socketNum <= 0){
		return -1;
	}

    if (timeoutMs >= 0) {
        // Calculate timeout value
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs - (tv.tv_sec * 1000)) * 1000;
        ptv = &tv;
    }

    FD_ZERO(&writefds);
    FD_SET(socketNum, &writefds);
    return select(socketNum + 1, NULL, &writefds, NULL, ptv);
}

AWE_DECLARE(int) awe_socket_setdscp(int socketNum, int iptos){
	iptos = (iptos << 2) & 0xFF;
	return setsockopt(socketNum, IPPROTO_IP, IP_TOS, &iptos, sizeof(iptos));
}

#if defined(_WIN32) && (_WIN32_WINNT < 0x0600)
const char* inet_ntop(int af, const void *src, char *dst, socklen_t size){
	struct sockaddr_storage ss;
	unsigned long s = size;

	ZeroMemory(&ss, sizeof(ss));
	ss.ss_family = af;

	switch (af) {
	case AF_INET:
		((struct sockaddr_in *)&ss)->sin_addr = *(struct in_addr *)src;
		break;
	case AF_INET6:
		((struct sockaddr_in6 *)&ss)->sin6_addr = *(struct in6_addr *)src;
		break;
	default:
		return NULL;
	}
	// cannot directly use &size because of strict aliasing rules
	return (WSAAddressToStringA((struct sockaddr *)&ss, sizeof(ss), NULL, dst, &s) == 0) ? dst : NULL;
}

int inet_pton(int af, const char *src, void *dst){
	struct sockaddr_storage ss;
	int size = sizeof(ss);
	char src_copy[INET6_ADDRSTRLEN + 1];

	ZeroMemory(&ss, sizeof(ss));
	// stupid non-const API
	strncpy(src_copy, src, INET6_ADDRSTRLEN + 1);
	src_copy[INET6_ADDRSTRLEN] = 0;

	if (WSAStringToAddressA(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0) {
		switch (af) {
		case AF_INET:
			*(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
			return 1;
		case AF_INET6:
			*(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
			return 1;
		}
	}
	return 0;
}
#endif
