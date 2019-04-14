/*
 * socketpair.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_SOCKETPAIR_H_
#define AWE_SOCKETPAIR_H_

#include <awe/awe.h>

AWE_BEGIN_DECLS

typedef struct awe_socketpeer{
	int* _sockfd;
}awe_socketpeer;

typedef struct awe_socketpair{
	int _fd[2];

	awe_socketpeer _sockpair[2];
}awe_socketpair;

awe_status_t awe_socketpair_init(awe_socketpair* sockpair, int type);

void awe_socketpair_deinit(awe_socketpair* sockpair);

awe_socketpeer* awe_socketpair_peer0(awe_socketpair* sockpair);
awe_socketpeer* awe_socketpair_peer1(awe_socketpair* sockpair);

int awe_socketpeer_fd(awe_socketpeer* sockpeer);
int awe_socketpeer_read(int fd, void* data, size_t size);
int awe_socketpeer_write(awe_socketpeer* sockpeer, const void* data, size_t size);

AWE_END_DECLS

#endif /* AWE_SOCKETPAIR_H_ */
