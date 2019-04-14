/*
 * packet.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define AWE_LOG_TAG "packet"
#include "awe/log.h"

#include "awe/atomic.h"
#include "awe/packet.h"

awe_packet* awe_packet_create(int32_t capacity){
	ALOG_ASSERT(capacity <= 0);
	awe_packet *packet = (awe_packet *)awe_mallocz(sizeof(awe_packet));
//	ALOGV("create(%p)", newpacket);

	awe_object_init(packet);
	packet->_rangeOffset = 0;
	packet->_rangeLength = capacity;

	packet->_dts = 0;
//	packet->_pts = 0;
	packet->_capacity = capacity;
	packet->_data = (uint8_t*)awe_mallocz(capacity);

	packet->_ownsData = TRUE;
	return packet;
}

static void packet_destroy(awe_packet *packet){
	if(packet->_ownsData == TRUE){
		awe_free(packet->_data);
	}

//	ALOGV("destroy(%p)", (packet));
	awe_free(packet);
}

awe_packet* awe_packet_addref(awe_packet *packet){
	awe_object_ref_inc(packet);
	return packet;
}

void awe_packet_autorelease(awe_packet **packet){
	if(packet == NULL || *packet == NULL){
		return ;
	}

	int32_t refs = awe_object_ref_dec(*packet);
	if(refs == 1){
		packet_destroy(*packet);
	}
	*packet = NULL;
}

void awe_packet_autorelease_proc(awe_object **e){
	awe_packet_autorelease((awe_packet **)e);
}

void awe_packet_setrange(awe_packet *packet, int32_t offset, int32_t size){
//    AWE_CHECK_LE(offset, packet->_capacity);
    AWE_CHECK_LE(offset + size, packet->_capacity);

    packet->_rangeOffset = offset;
    packet->_rangeLength = size;
}

void awe_packet_setrange2(awe_packet *packet, int32_t offset, int32_t size){
    packet->_rangeOffset += offset;
    packet->_rangeLength = size;

//    AWE_CHECK_LE(packet->_rangeOffset, packet->_capacity);
    AWE_CHECK_LE(packet->_rangeOffset + size, packet->_capacity);
}

void awe_packet_setrange3(awe_packet *packet, int32_t offset, int32_t size){
    packet->_rangeOffset = offset;
    packet->_rangeLength += size;

//    AWE_CHECK_LE(packet->_rangeOffset, packet->_capacity);
    AWE_CHECK_LE(packet->_rangeOffset + packet->_rangeLength, packet->_capacity);
}
