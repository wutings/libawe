/*
 * socketpair.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#define AWE_LOG_TAG "socketpair"
#include "awe/log.h"

#include "awe/socket_helper.h"

#include "awe/socketpair.h"

#if defined(__WIN32__) || defined(_WIN32)
#define LOCAL_SOCKETPAIR_AF AF_INET
#else
#define LOCAL_SOCKETPAIR_AF AF_UNIX
#define socketpair_create socketpair
#endif

#define AWE_SOCKPEER_0 0
#define AWE_SOCKPEER_1 1

awe_status_t awe_socketpair_init(awe_socketpair* sockpair, int type){
//	ALOGV("awe_socketpair_init(%p)", sockpair);
	AWE_CHECK_EQ(0, socketpair_create(LOCAL_SOCKETPAIR_AF, type, 0, sockpair->_fd));

	awe_socket_make_nonblocking(sockpair->_fd[AWE_SOCKPEER_0]);
	awe_socket_make_nonblocking(sockpair->_fd[AWE_SOCKPEER_1]);

	sockpair->_sockpair[AWE_SOCKPEER_0]._sockfd = &sockpair->_fd[AWE_SOCKPEER_0];
	sockpair->_sockpair[AWE_SOCKPEER_1]._sockfd = &sockpair->_fd[AWE_SOCKPEER_1];

	return AWE_OK;
}

void awe_socketpair_deinit(awe_socketpair* sockpair){
//	ALOGV("awe_socketpair_deinit(%p)", sockpair);
	if(sockpair->_fd[AWE_SOCKPEER_0] > 0){
		socket_close(sockpair->_fd[AWE_SOCKPEER_0]);
		sockpair->_fd[AWE_SOCKPEER_0] = -1;
	}

	if(sockpair->_fd[AWE_SOCKPEER_1] > 0){
		socket_close(sockpair->_fd[AWE_SOCKPEER_1]);
		sockpair->_fd[AWE_SOCKPEER_1] = -1;
	}
}

awe_socketpeer* awe_socketpair_peer0(awe_socketpair* sockpair){
	return &sockpair->_sockpair[AWE_SOCKPEER_0];
}

awe_socketpeer* awe_socketpair_peer1(awe_socketpair* sockpair){
	return &sockpair->_sockpair[AWE_SOCKPEER_1];
}

int awe_socketpeer_fd(awe_socketpeer* sockpeer){
	return *sockpeer->_sockfd;
}

int awe_socketpeer_read(int fd, void* data, size_t size){
	return awe_socket_read(fd, data, size);
}

int awe_socketpeer_write(awe_socketpeer* sockpeer, const void* data, size_t size){
	return awe_socket_write(*sockpeer->_sockfd, data, size);
}
