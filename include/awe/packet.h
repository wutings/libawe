/*
 * packet.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_PACKET_H_
#define AWE_PACKET_H_

#include <stdint.h>
#include <stddef.h>

#include <awe/awe.h>
#include <awe/object.h>

AWE_BEGIN_DECLS

typedef struct awe_packet{
	AWE_OBJECT_DEC;

    int32_t _rangeOffset;
    int32_t _rangeLength;

    uint32_t _dts;

    int32_t _capacity;

    uint8_t *_data;

    bool _ownsData;
}awe_packet;


awe_packet* awe_packet_create(int32_t capacity);

awe_packet* awe_packet_addref(awe_packet *packet);
void awe_packet_autorelease(awe_packet **packet);

void awe_packet_autorelease_proc(awe_object **o);

static inline uint8_t *awe_packet_base(awe_packet *packet) {
	return packet->_data;
}
static inline uint8_t *awe_packet_data(awe_packet *packet) {
	return packet->_data + packet->_rangeOffset;
}
static inline int32_t awe_packet_capacity(awe_packet *packet) {
	return packet->_capacity;
}
static inline int32_t awe_packet_size(awe_packet *packet) {
	return packet->_rangeLength;
}
static inline int32_t awe_packet_offset(awe_packet *packet) {
	return packet->_rangeOffset;
}
static inline uint32_t awe_packet_getdts(awe_packet *packet){
	return packet->_dts;
}
static inline void awe_packet_setdts(awe_packet *packet, uint32_t dts) {
	packet->_dts = dts;
}

void awe_packet_setrange(awe_packet *packet, int32_t offset, int32_t size);
void awe_packet_setrange2(awe_packet *packet, int32_t offset, int32_t size);
void awe_packet_setrange3(awe_packet *packet, int32_t offset, int32_t size);

AWE_END_DECLS

#endif /* AWE_PACKET_H_ */
