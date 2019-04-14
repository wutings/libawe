/*
 * sl_ux_dummy.c
 *
 */

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#include <ux.h>

int __ufd_set_set(UDTSOCKET u, ufd_set *set){
	return -1;
}
void __ufd_set_clr(UDTSOCKET u, ufd_set *set){
}
int __ufd_set_isset(UDTSOCKET u, ufd_set *set){
	return 0;
}

UX_API int ux_startup(){
	return -1;
}
UX_API int ux_cleanup(){
	return -1;
}
UX_API UDTSOCKET ux_socket(int af, int type, int protocol){
	return -1;
}
UX_API int ux_bind(UDTSOCKET u, const struct sockaddr* name, int namelen){
	return -1;
}
UX_API int ux_bind2(UDTSOCKET u, UDPSOCKET udpsock){
	return -1;
}
UX_API int ux_listen(UDTSOCKET u, int backlog){
	return -1;
}
UX_API UDTSOCKET ux_accept(UDTSOCKET u, struct sockaddr* addr, int* addrlen){
	return -1;
}
UX_API int ux_connect(UDTSOCKET u, const struct sockaddr* name, int namelen){
	return -1;
}
UX_API int ux_close(UDTSOCKET u){
	return -1;
}
UX_API int ux_getpeername(UDTSOCKET u, struct sockaddr* name, int* namelen){
	return -1;
}
UX_API int ux_getsockname(UDTSOCKET u, struct sockaddr* name, int* namelen){
	return -1;
}
UX_API int ux_getsockopt(UDTSOCKET u, int level, uxopt optname, void* optval, int* optlen){
	return -1;
}
UX_API int ux_setsockopt(UDTSOCKET u, int level, uxopt optname, const void* optval, int optlen){
	return -1;
}
UX_API int ux_send(UDTSOCKET u, const char* buf, int len, int flags){
	return -1;
}
UX_API int ux_recv(UDTSOCKET u, char* buf, int len, int flags){
	return -1;
}
UX_API int ux_sendmsg(UDTSOCKET u, const char* buf, int len, int ttl/* = -1*/, bool inorder/* = false*/){
	return -1;
}
UX_API int ux_recvmsg(UDTSOCKET u, char* buf, int len){
	return -1;
}

UX_API int ux_select(int nfds, ufd_set* readfds, ufd_set* writefds, ufd_set* exceptfds, const struct timeval* timeout){
	return -1;
}

UX_API int reg_udp_recvcb(UDPSOCKET udpsock, void (*recvcb)(void*, int, struct sockaddr*, void*), void* arg){
	return -1;
}

UX_API int ux_connected(UDTSOCKET u){
	return -1;
}
