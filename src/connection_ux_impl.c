/*
 * connection_ux_impl.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include <ux.h>

#define AWE_LOG_TAG "connection_ux_impl"
#include "awe/log.h"

#include "awe/connection.h"
#include "connection_internal.h"

static awe_status_t ux_stream_close(awe_connection_t *conn) {
	if (conn->_fd > 0) {
		ux_close(conn->_fd);
		conn->_fd = -1;
		ALOGV("id:%u, close.", conn->_id);
	}
	return AWE_OK;
}

static awe_status_t conn_ux_open(awe_connection_t *conn, int type, uint16_t bindPort){
	if (conn->_fd > 0){
		ALOGV("id:%u, connection have opened.", conn->_id);
		return AWE_OK;
	}

	do{
		int udpfd = awe_connection_fd(conn->_udpconn);
		if(udpfd <= 0){
			ALOGE("id:%u, udpconn not open.", conn->_id);
			break;
		}
		conn->_fd = ux_socket(conn->_saddr._af, type, 0);

		if(ux_bind2(conn->_fd, udpfd) < 0){
			ALOGE("id:%u, ux bind2 error.", conn->_id);
			break;
		}

		char blocking = 1;
		ux_setsockopt(conn->_fd, 0, UDT_SNDSYN, (char*)&blocking, sizeof(blocking));
		ux_setsockopt(conn->_fd, 0, UDT_RCVSYN, (char*)&blocking, sizeof(blocking));

		ux_connect(conn->_fd, (struct sockaddr*)conn->_saddr._addr, conn->_saddr._addrlen);
		ALOGV("id:%u, opened.", conn->_id);
		return AWE_OK;
	}while(0);
	ux_stream_close(conn);
	return -1;
}

static awe_status_t ux_stream_open(awe_connection_t *conn, const char *host, uint16_t port, uint16_t bindPort) {
	if(conn->_fd <= 0 && awe_socket_addrinfo(&conn->_saddr, host, port) == AWE_OK){
		return conn_ux_open(conn, SOCK_STREAM, bindPort);
	}
	return -1;
}

static awe_status_t ux_stream_open2(awe_connection_t *conn, struct sockaddr *addr, socklen_t addrlen, uint16_t bindPort){
	return -1;
}

static int ux_stream_read(awe_connection_t *conn, void* buffer, size_t size) {
	return ux_recv(conn->_fd, (char*)buffer, size, 0);
}

static int ux_stream_read2(awe_connection_t *conn,
		void* buffer, size_t size, struct sockaddr *fromAddress) {
	return -1;
}

static int ux_stream_write(awe_connection_t *conn, const void* buffer, size_t size) {
	return ux_send(conn->_fd, (char*)buffer, size, 0);
}

static int ux_stream_write2(awe_connection_t *conn,
		const char *host, uint16_t port, const void* buffer, size_t size) {
	return awe_connection_write2(conn->_udpconn, host, port, buffer, size);
}

struct awe_conn_interface ux_stream = {
	.open	= ux_stream_open,
	.open2	= ux_stream_open2,
	.close 	= ux_stream_close,
	.read 	= ux_stream_read,
	.read2 	= ux_stream_read2,
	.write 	= ux_stream_write,
	.write2 = ux_stream_write2,
};

static awe_status_t ux_dgram_open(awe_connection_t *conn, const char *host, uint16_t port, uint16_t bindPort) {
	if(conn->_fd <= 0 && awe_socket_addrinfo(&conn->_saddr, host, port) == AWE_OK){
		return conn_ux_open(conn, SOCK_DGRAM, bindPort);
	}
	return -1;
}

static awe_status_t ux_dgram_open2(awe_connection_t *conn, struct sockaddr *addr, socklen_t addrlen, uint16_t bindPort){
	if(conn->_fd <= 0){
		conn->_saddr._addrlen = addrlen;
		memcpy(conn->_saddr._addr, addr, addrlen);
		return conn_ux_open(conn, SOCK_DGRAM, bindPort);
	}
	return -1;
}

static int ux_dgram_read(awe_connection_t *conn, void* buffer, size_t size) {
	return ux_recvmsg(conn->_fd, (char*)buffer, size);
}

static int ux_dgram_read2(awe_connection_t *conn,
		void* buffer, size_t size, struct sockaddr *fromAddress) {
	return -1;
}

static int ux_dgram_write(awe_connection_t *conn, const void* buffer, size_t size) {
	return ux_sendmsg(conn->_fd, (char*)buffer, size, -1, false);
}

static int ux_dgram_write2(awe_connection_t *conn,
		const char *host, uint16_t port, const void* buffer, size_t size) {
	return awe_connection_write2(conn->_udpconn, host, port, buffer, size);
}

awe_conn_interface ux_dgram = {
	.open	= ux_dgram_open,
	.open2	= ux_dgram_open2,
	.close	= ux_stream_close,
	.read 	= ux_dgram_read,
	.read2 	= ux_dgram_read2,
	.write 	= ux_dgram_write,
	.write2 = ux_dgram_write2,
};
