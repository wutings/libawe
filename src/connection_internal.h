/*
 * connection_internal.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_CONNECTION_INTERNAL_H_
#define AWE_CONNECTION_INTERNAL_H_

#include "awe/object.h"

typedef struct awe_conn_interface{
	awe_status_t (*open)(awe_connection_t *conn, const char *host, uint16_t port, uint16_t bindPort);
	awe_status_t (*open2)(awe_connection_t *conn, struct sockaddr *addr, socklen_t addrlen, uint16_t bindPort);
	awe_status_t (*close)(awe_connection_t *conn);

    int (*read)(awe_connection_t *conn, void *buffer, size_t size);
    int (*read2)(awe_connection_t *conn, void *buffer, size_t size, struct sockaddr *fromAddress);

    int (*write)(awe_connection_t *conn, const void *buffer, size_t size);
    int (*write2)(awe_connection_t *conn, const char *host, uint16_t port,
    		const void *buffer, size_t size);
}awe_conn_interface;

struct awe_connection{
	AWE_OBJECT_DEC;

	uint32_t	_id;
	int			_fd; /** socket descriptor */

	socket_addr _saddr;

	uint16_t	_local_port;

	awe_conn_interface*	_impl;
	awe_connection_t*	_udpconn;
};

#endif /* AWE_CONNECTION_INTERNAL_H_ */
