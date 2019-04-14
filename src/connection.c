/*
 * connection.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#define AWE_LOG_TAG "connection"
#include "awe/log.h"

#include "awe/awe.h"
#include "awe/connection.h"
#include "connection_internal.h"

extern awe_conn_interface inet_stream;

extern awe_conn_interface inet4_dgram;
extern awe_conn_interface inet6_dgram;

extern awe_conn_interface ux_stream;
extern awe_conn_interface ux_dgram;

awe_connection_t* awe_connection_stream(int af, int fd, struct sockaddr* addr, socklen_t addrlen, uint32_t id){
	awe_connection_t *new_conn = (awe_connection_t *)awe_mallocz(sizeof(awe_connection_t));
	ALOGV_IF(new_conn == NULL, "create stream(%p), id:%u.", new_conn, id);
	awe_object_init(new_conn);

	new_conn->_id = id;
	if(fd != 0){
		new_conn->_fd = fd;
		new_conn->_saddr._addrlen = addrlen;
		memcpy(new_conn->_saddr._addr, addr, addrlen);
	}
	new_conn->_saddr._af = af;

	new_conn->_impl = &inet_stream;
	return new_conn;
}

awe_connection_t* awe_connection_stream2(int af, uint32_t id){
	return awe_connection_stream(af, 0, NULL, 0, id);
}

awe_connection_t* awe_connection_dgram(int af, int fd, uint32_t id){
	awe_connection_t *new_conn = (awe_connection_t *)awe_mallocz(sizeof(awe_connection_t));
	ALOGV_IF(new_conn == NULL, "create dgram(%p), id:%u.", new_conn, id);
	awe_object_init(new_conn);

	new_conn->_id = id;
	new_conn->_fd = fd;
	new_conn->_saddr._af = af;

	if(AF_INET == af){
		new_conn->_impl = &inet4_dgram;
	}else if(AF_INET6 == af){
		new_conn->_impl = &inet6_dgram;
	}else{
		awe_connection_autorelease(&new_conn);
	}
	return new_conn;
}

awe_connection_t* awe_connection_dgram2(int af, uint32_t id){
	return awe_connection_dgram(af, 0, id);
}

awe_connection_t* awe_connection_ux(int type, int ufd,
		struct sockaddr* addr, socklen_t addrlen, awe_connection_t *bindconn, uint32_t id){
	awe_connection_t *new_conn = (awe_connection_t *)awe_mallocz(sizeof(awe_connection_t));
	ALOGV_IF(new_conn == NULL, "id:%u, create ux(%p), type:%d", id, new_conn, type);
	awe_object_init(new_conn);

	new_conn->_id = id;
	if(ufd != 0){
		new_conn->_fd = ufd;
		new_conn->_saddr._addrlen = addrlen;
		memcpy(new_conn->_saddr._addr, addr, addrlen);
	}
	new_conn->_saddr._af = bindconn->_saddr._af;

	if(SOCK_STREAM == type){
		new_conn->_impl = &ux_stream;
	}else{
		new_conn->_impl = &ux_dgram;
	}

	new_conn->_udpconn = awe_connection_addref(bindconn);

	return new_conn;
}

awe_connection_t* awe_connection_ux2(int type, awe_connection_t *bindconn, uint32_t id){
	return awe_connection_ux(type, 0, NULL, 0, bindconn, id);
}

awe_connection_t* awe_connection_addref(awe_connection_t *conn){
	awe_object_ref_inc(conn);
	return conn;
}

static void connection_destroy(awe_connection_t *conn){
	awe_connection_close(conn);

	if(conn->_udpconn != NULL){
		awe_connection_autorelease(&conn->_udpconn);
	}

	ALOGV("id:%u, destroy(%p)", conn->_id, conn);
	awe_free(conn);
}

void awe_connection_autorelease(awe_connection_t **conn){
	if(conn == NULL || *conn == NULL){
		return ;
	}

	int32_t refs = awe_object_ref_dec(*conn);
	if(refs == 1){
		connection_destroy(*conn);
	}
	*conn = NULL;
}

awe_status_t awe_connection_open(awe_connection_t *conn, const char *host, uint16_t port, uint16_t bindPort){
	return conn->_impl->open(conn, host, port, bindPort);
}

awe_status_t awe_connection_open2(awe_connection_t *conn, struct sockaddr *addr, socklen_t addrlen, uint16_t bindPort){
	return conn->_impl->open2(conn, addr, addrlen, bindPort);
}

awe_status_t awe_connection_close(awe_connection_t *conn){
	return conn->_impl->close(conn);
}

int awe_connection_read(awe_connection_t *conn, void *buffer, size_t size){
	return conn->_impl->read(conn, buffer, size);
}

int awe_connection_read2(awe_connection_t *conn,
		void *buffer, size_t size, struct sockaddr *fromAddress){
	return conn->_impl->read2(conn, buffer, size, fromAddress);
}

int awe_connection_write(awe_connection_t *conn, const void *buffer, size_t size){
	return conn->_impl->write(conn, buffer, size);
}

int awe_connection_write2(awe_connection_t *conn,
		const char *host, uint16_t port, const void *buffer, size_t size){
	return conn->_impl->write2(conn, host, port, buffer, size);
}

void awe_connection_setid(awe_connection_t *conn, uint32_t id){
	conn->_id = id;
}

int awe_connection_fd(awe_connection_t *conn){
	return conn->_fd;
}

int awe_connection_af(awe_connection_t *conn){
	return conn->_saddr._af;
}

bool awe_connection_isUX(awe_connection_t *conn){
	return conn->_udpconn != NULL;
}

struct sockaddr* awe_connection_addr(awe_connection_t *conn){
	return (struct sockaddr*)conn->_saddr._addr;
}
socklen_t awe_connection_addrlen(awe_connection_t *conn){
	return conn->_saddr._addrlen;
}

awe_status_t awe_connection_localHost(awe_connection_t *conn, char *buf, int buflen){
	return awe_socket_localHost(conn->_saddr._af, conn->_fd, buf, buflen);
}

uint16_t awe_connection_localPort(awe_connection_t *conn){
	if(conn->_local_port == 0){
		awe_socket_localPort(conn->_saddr._af, conn->_fd, &conn->_local_port);
	}
	return conn->_local_port;
}

awe_status_t awe_connection_remoteHost(awe_connection_t *conn, char* host, int len){
	uint16_t fromPort = 0;
	return awe_socket_inet_ntop(conn->_saddr._af, (struct sockaddr*)&conn->_saddr._addr, host, len, &fromPort);
}

uint16_t awe_connection_remotePort(awe_connection_t *conn){
	char host[INET6_ADDRSTRLEN] = {0};
	uint16_t port = 0;
	awe_socket_inet_ntop(conn->_saddr._af, (struct sockaddr*)&conn->_saddr._addr, host, sizeof(host), &port);
	return port;
}
