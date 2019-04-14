/*
 * connection.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_CONNECTION_H_
#define AWE_CONNECTION_H_

#include <awe/awe.h>

AWE_BEGIN_DECLS

typedef enum ConnType{
	AWE_CONN_STREAM = 1,
	AWE_CONN_DGRAM,
	AWE_CONN_UX_STREAM,
	AWE_CONN_UX_DGRAM,

	AWE_CONN_V6_UX_STREAM,
	AWE_CONN_V6_UX_DGRAM,
}ConnType;

typedef enum awe_af_t{
	AWE_STREAM			= SOCK_STREAM,
	AWE_DGRAM			= SOCK_DGRAM,
	AWE_UX_STREAM		,
	AWE_UX_DGRAM		,

	AWE_STREAM_V6		= SOCK_STREAM,
	AWE_DGRAM_V6		= SOCK_DGRAM,
	AWE_UX_STREAM_V6,
	AWE_UX_DGRAM_V6,
}awe_af_t;

typedef struct awe_connection awe_connection_t;

/*
 * Create a connection.
 * @return 0: success,
 * 		 < 0: failed.
 */
awe_connection_t* awe_connection_stream(int af, int fd,
		struct sockaddr* addr, socklen_t addrlen, uint32_t id);
awe_connection_t* awe_connection_stream2(int af, uint32_t id);

awe_connection_t* awe_connection_dgram(int af, int fd, uint32_t id);
awe_connection_t* awe_connection_dgram2(int af, uint32_t id);


awe_connection_t* awe_connection_ux(int type, int ufd,
		struct sockaddr* addr, socklen_t addrlen, awe_connection_t *bindconn, uint32_t id);
awe_connection_t* awe_connection_ux2(int type, awe_connection_t *bindconn, uint32_t id);


awe_connection_t* awe_connection_addref(awe_connection_t *conn);

/**
 * Destroy the awe_connection_t variable and free the associated memory.
 * @param conn the awe_connection_t variable to destroy.
 */
void awe_connection_autorelease(awe_connection_t **conn);

/**
 * Open connection
 * @return 0: The connection is pending
 * 		  1: local connection connected
 * 		  > 1: Connection had opened
 * 		  < 0: error
 */
awe_status_t awe_connection_open(awe_connection_t *conn, const char *host, uint16_t port, uint16_t bindPort);

awe_status_t awe_connection_open2(awe_connection_t *conn, struct sockaddr *addr, socklen_t addrlen, uint16_t bindPort);

/*
 * Close connection
 */
awe_status_t awe_connection_close(awe_connection_t *conn);

/**
 * Read bytes from the connection.
 *
 * @param buffer    Pointer to destination buffer.
 * @param size       Number of bytes to read.
 * @return Number of bytes read.
 * 			< 0 if error.
 */
int awe_connection_read(awe_connection_t *conn,
		void *buffer, size_t size);

int awe_connection_read2(awe_connection_t *conn,
		void *buffer, size_t size, struct sockaddr *fromAddress);

/**
 * Write bytes to the connection.
 *
 * @param buffer    Pointer to source buffer.
 * @param size       Number of bytes to read.
 * @return Number of bytes written.
 * 			< 0 if error.
 */
int awe_connection_write(awe_connection_t *conn,
		const void *buffer, size_t size);

int awe_connection_write2(awe_connection_t *conn,
		const char *host, uint16_t port, const void *buffer, size_t size);

void awe_connection_setid(awe_connection_t *conn, uint32_t id);

int awe_connection_fd(awe_connection_t *conn);
int awe_connection_af(awe_connection_t *conn);

bool awe_connection_isUX(awe_connection_t *conn);

struct sockaddr* awe_connection_addr(awe_connection_t *conn);
socklen_t awe_connection_addrlen(awe_connection_t *conn);

awe_status_t awe_connection_localHost(awe_connection_t *conn, char *buf, int buflen);
uint16_t awe_connection_localPort(awe_connection_t *conn);

awe_status_t awe_connection_remoteHost(awe_connection_t *conn, char* host, int len);
uint16_t awe_connection_remotePort(awe_connection_t *conn);

AWE_END_DECLS

#endif /* AWE_CONNECTION_H_ */
