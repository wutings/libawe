/*
 * connection_impl.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#define AWE_LOG_TAG "connection_impl"
#include "awe/log.h"

#include "awe/connection.h"
#include "connection_internal.h"


static awe_status_t inet_stream_close(awe_connection_t *conn){
	ALOGI("id:%u, inet_stream_close(%d).", conn->_id, conn->_fd);
	if (conn->_fd > 0) {
		socket_close(conn->_fd);
		conn->_fd = -1;
	}
	return AWE_OK;
}

static awe_status_t inet_stream_open(awe_connection_t *conn, const char *host, uint16_t port, uint16_t bindPort) {
	if (conn->_fd > 0){
		ALOGI("id:%u, connection have opened.", conn->_id);
		return AWE_OK;
	}

	do{
		if(awe_socket_addrinfo(&conn->_saddr, host, port) != AWE_OK){
			break;
		}

		conn->_fd = awe_socket_stream_setup(conn->_saddr._af, bindPort, true);
		if(conn->_fd <= 0){
			ALOGE("id:%u, setup error(%d)", conn->_id, S_ERRNO);
			break;
		}
		awe_socket_ignore_sigpipe(conn->_fd);//so that servers on the same host that get killed don't also kill us

		if(connect(conn->_fd, (struct sockaddr*)conn->_saddr._addr, conn->_saddr._addrlen) != 0){
			int const err = S_ERRNO;
			if(err == EINPROGRESS || err == EWOULDBLOCK || err == EINTR){
				//The connection is pending; we'll need to handle it later.
				//Wait for our socket to be 'britable', or have an exception.
			}else{
				ALOGE("id:%u, connect failed(%d).", conn->_id, err);
				break;
			}
		}
		ALOGI("id:%u, opened(%d).", conn->_id, conn->_fd);
		return AWE_OK;
	}while(0);
	inet_stream_close(conn);
	return -1;
}

static awe_status_t inet_stream_open2(awe_connection_t *conn, struct sockaddr *addr, socklen_t addrlen, uint16_t bindPort){
	return -1;
}

static int inet_stream_read(awe_connection_t *conn, void* buffer, size_t size) {
	return awe_socket_read(conn->_fd, buffer, size);
}

static int inet_stream_read2(awe_connection_t *conn,
		void* buffer, size_t size, struct sockaddr *fromAddress) {
	return -1;
}

static int inet_stream_write(awe_connection_t *conn, const void* buffer, size_t size) {
	return awe_socket_write(conn->_fd, buffer, size);
}

static int inet_stream_write2(awe_connection_t *conn,
		const char *host, uint16_t port, const void *buffer, size_t size) {
	return -1;
}

awe_conn_interface inet_stream = {
	.open	= inet_stream_open,
	.open2	= inet_stream_open2,
	.close 	= inet_stream_close,
	.read 	= inet_stream_read,
	.read2 	= inet_stream_read2,
	.write 	= inet_stream_write,
	.write2 = inet_stream_write2,
};

//------------------------------------------------------------------------------------

static awe_status_t inet_dgram_close(awe_connection_t *conn) {
	ALOGI("id:%u, inet_dgram_close(%d).", conn->_id, conn->_fd);
	if (conn->_fd > 0) {
		socket_close(conn->_fd);
		conn->_fd = -1;
	}
	return AWE_OK;
}

static awe_status_t inet_dgram_open(awe_connection_t* conn, const char *host, uint16_t port, uint16_t bindPort) {
	if (conn->_fd > 0){
		ALOGV("id:%u, connection have opened.", conn->_id);
		return AWE_OK;
	}

	do{
		if(host != NULL && strlen(host) > 0){
			if(awe_socket_addrinfo(&conn->_saddr, host, port) != AWE_OK){
				break;
			}
		}

		conn->_fd = awe_socket_datagram_setup(conn->_saddr._af, bindPort, true);
		if(conn->_fd <= 0){
			ALOGE("id:%u, setup error(%d)", conn->_id, S_ERRNO);
			break;
		}

		awe_socket_ignore_sigpipe(conn->_fd);//so that servers on the same host that get killed don't also kill us
		return AWE_OK;
	}while(0);
	inet_dgram_close(conn);
	return -1;
}

static int inet4_dgram_read(awe_connection_t *conn, void* buffer, size_t size) {
	struct sockaddr_in dummy;
	socklen_t len = sizeof(struct sockaddr_in);
	return awe_socket_recvfrom(conn->_fd, buffer, size, (struct sockaddr*)&dummy, &len);
}

static int inet4_dgram_read2(awe_connection_t *conn,
		void* buffer, size_t size, struct sockaddr *fromAddress) {
	socklen_t len = sizeof(struct sockaddr_in);
	return awe_socket_recvfrom(conn->_fd, buffer, size, fromAddress, &len);
}

static int inet_dgram_write(awe_connection_t *conn, const void* buffer, size_t size) {
	return awe_socket_sendto(conn->_fd, buffer, size, (struct sockaddr*)conn->_saddr._addr, conn->_saddr._addrlen);
}

static int inet4_dgram_write2(awe_connection_t *conn,
		const char *host, uint16_t port, const void* buffer, size_t size) {
	struct sockaddr_in dest;
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    if(inet_pton(AF_INET, host, &dest.sin_addr) > 0){
    	return awe_socket_sendto(conn->_fd, buffer, size, (struct sockaddr*)&dest, sizeof(struct sockaddr_in));
    }
    ALOGE("id:%u, write2 error, Not in presentation format:\"%s\".", conn->_id, host);
    return -1;
}

awe_conn_interface inet4_dgram = {
	.open	= inet_dgram_open,
	.open2	= inet_stream_open2,
	.close	= inet_dgram_close,
	.read 	= inet4_dgram_read,
	.read2 	= inet4_dgram_read2,
	.write 	= inet_dgram_write,
	.write2 = inet4_dgram_write2,
};

static int inet6_dgram_read(awe_connection_t *conn, void* buffer, size_t size) {
	struct sockaddr_in6 dummy;
	socklen_t len = sizeof(struct sockaddr_in6);
	return awe_socket_recvfrom(conn->_fd, buffer, size, (struct sockaddr*)&dummy, &len);
}

static int inet6_dgram_read2(awe_connection_t *conn,
		void* buffer, size_t size, struct sockaddr *fromAddress) {
	socklen_t len = sizeof(struct sockaddr_in6);
	return awe_socket_recvfrom(conn->_fd, buffer, size, fromAddress, &len);
}

static int inet6_dgram_write2(awe_connection_t *conn,
		const char *host, uint16_t port, const void* buffer, size_t size){
	struct sockaddr_in6 dest;
    bzero(&dest, sizeof(dest));
    dest.sin6_family = AF_INET6;
    dest.sin6_port = htons(port);
    if(inet_pton(AF_INET6, host, &dest.sin6_addr) > 0){
    	return awe_socket_sendto(conn->_fd, buffer, size, (struct sockaddr*)&dest, sizeof(struct sockaddr_in6));
    }
    ALOGE("id:%u, write2 error, Not in presentation format:\"%s\".", conn->_id, host);
    return -1;
}

awe_conn_interface inet6_dgram = {
	.open	= inet_dgram_open,
	.open2	= inet_stream_open2,
	.close	= inet_dgram_close,
	.read 	= inet6_dgram_read,
	.read2 	= inet6_dgram_read2,
	.write 	= inet_dgram_write,
	.write2 = inet6_dgram_write2,
};
