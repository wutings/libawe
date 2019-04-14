/*
 * ux.h
 *
 *  Created on: May 29, 2016
 *      Author: root
 */

#ifndef UX_H_
#define UX_H_

#include <stdbool.h>
#include <sys/socket.h>

#define UX_API

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
   #ifndef __MINGW__
      typedef SOCKET SYSSOCKET;
   #else
      typedef int SYSSOCKET;
   #endif
#else
   typedef int SYSSOCKET;
#endif

typedef SYSSOCKET UDPSOCKET;
typedef int UDTSOCKET;

typedef struct ufd_set{
	UDTSOCKET *ufd_arr;
	int length;
}ufd_set;

int __ufd_set_set(UDTSOCKET u, ufd_set *set);
void __ufd_set_clr(UDTSOCKET u, ufd_set *set);
int __ufd_set_isset(UDTSOCKET u, ufd_set *set);

#define UFD_SET_DECLARE(name, len) \
UDTSOCKET __##name##_arr[len]; \
ufd_set name = {.ufd_arr = (UDTSOCKET*)&__##name##_arr[0], .length = len}

#define UFD_ZERO(ufdsetp) memset((ufdsetp)->ufd_arr, 0, sizeof(UDTSOCKET) * (ufdsetp)->length)
#define	UFD_SET(ufd, ufdsetp) __ufd_set_set(ufd, ufdsetp)
#define	UFD_CLR(ufd, ufdsetp) __ufd_set_clr(ufd, ufdsetp)
#define	UFD_ISSET(ufd, ufdsetp) __ufd_set_isset(ufd, ufdsetp)


//-----------------------------------------------------------------------------

enum UDTSTATUS {INIT = 1, OPENED, LISTENING, CONNECTING, CONNECTED, BROKEN, CLOSING, CLOSED, NONEXIST};

////////////////////////////////////////////////////////////////////////////////

enum UDTOpt
{
   UDT_MSS,             // the Maximum Transfer Unit
   UDT_SNDSYN,          // if sending is blocking
   UDT_RCVSYN,          // if receiving is blocking
   UDT_CC,              // custom congestion control algorithm
   UDT_FC,				// Flight flag size (window size)
   UDT_SNDBUF,          // maximum buffer in sending queue
   UDT_RCVBUF,          // UDT receiving buffer size
   UDT_LINGER,          // waiting for unsent data when closing
   UDP_SNDBUF,          // UDP sending buffer size
   UDP_RCVBUF,          // UDP receiving buffer size
   UDT_MAXMSG,          // maximum datagram message size
   UDT_MSGTTL,          // time-to-live of a datagram message
   UDT_RENDEZVOUS,      // rendezvous connection mode
   UDT_SNDTIMEO,        // send() timeout
   UDT_RCVTIMEO,        // recv() timeout
   UDT_REUSEADDR,	// reuse an existing port or create a new one
   UDT_MAXBW,		// maximum bandwidth (bytes per second) that the connection can use
   UDT_STATE,		// current socket state, see UDTSTATUS, read only
   UDT_EVENT,		// current avalable events associated with the socket
   UDT_SNDDATA,		// size of data in the sending buffer
   UDT_RCVDATA		// size of data available for recv
};
typedef enum UDTOpt uxopt;

UX_API int ux_startup();
UX_API int ux_cleanup();
UX_API UDTSOCKET ux_socket(int af, int type, int protocol);
UX_API int ux_bind(UDTSOCKET u, const struct sockaddr* name, int namelen);
UX_API int ux_bind2(UDTSOCKET u, UDPSOCKET udpsock);
UX_API int ux_listen(UDTSOCKET u, int backlog);
UX_API UDTSOCKET ux_accept(UDTSOCKET u, struct sockaddr* addr, int* addrlen);
UX_API int ux_connect(UDTSOCKET u, const struct sockaddr* name, int namelen);
UX_API int ux_close(UDTSOCKET u);
UX_API int ux_getpeername(UDTSOCKET u, struct sockaddr* name, int* namelen);
UX_API int ux_getsockname(UDTSOCKET u, struct sockaddr* name, int* namelen);
UX_API int ux_getsockopt(UDTSOCKET u, int level, uxopt optname, void* optval, int* optlen);
UX_API int ux_setsockopt(UDTSOCKET u, int level, uxopt optname, const void* optval, int optlen);
UX_API int ux_send(UDTSOCKET u, const char* buf, int len, int flags);
UX_API int ux_recv(UDTSOCKET u, char* buf, int len, int flags);
UX_API int ux_sendmsg(UDTSOCKET u, const char* buf, int len, int ttl/* = -1*/, bool inorder/* = false*/);
UX_API int ux_recvmsg(UDTSOCKET u, char* buf, int len);

UX_API int ux_select(int nfds, ufd_set* readfds, ufd_set* writefds, ufd_set* exceptfds, const struct timeval* timeout);

UX_API int reg_udp_recvcb(UDPSOCKET udpsock, void (*recvcb)(void*, int, struct sockaddr*, void*), void* arg);

UX_API int ux_connected(UDTSOCKET u);

#ifdef __cplusplus
}
#endif

#endif /* UX_H_ */
