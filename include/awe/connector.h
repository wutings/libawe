/*
 * connector.h
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#ifndef AWE_CONNECTOR_H_
#define AWE_CONNECTOR_H_

#include <awe/connection.h>
#include <awe/scheduler.h>

#include <awe/uxscheduler.h>

AWE_BEGIN_DECLS

typedef void connector_connectedhandler(void* userdata, int32_t result, int fd);

typedef struct awe_connector{
	awe_scheduler_t* _sched;
	awe_connection_t* _conn; //tcp connection
	awe_watchid _conntoken;
	awe_taskid _timeouttoken;

	connector_connectedhandler* _onconnected_proc;
	void* _userdata;
}awe_connector;

awe_connector* awe_connector_create(awe_scheduler_t* sched,
		connector_connectedhandler* connhandler, void* userdata);

void awe_connector_destroy(awe_connector *connector);

awe_status_t awe_connector_open(awe_connector *connector, const char* ip, uint16_t port,
		int32_t timeout, uint32_t id);

awe_status_t awe_connector_open2(awe_connector *connector, awe_connection_t* tcpconn);

void awe_connector_close(awe_connector *connector);

awe_connection_t* awe_connector_takeout(awe_connector *connector);

void awe_connector_setEventHandler(awe_connector *connector, int fd,
		sockethandler_proc* proc, void* userdata);

void awe_connector_rmEventHandler(awe_connector *connector);

//===============================================================================

typedef struct awe_uxconnector{
	awe_uxscheduler_t* _uxsched;
	awe_connection_t* _uxconn; //UX connection

	awe_uxsched_task_id _timeouttask_id;

	connector_connectedhandler* _onconnected_proc;
	void* _userdata;
}awe_uxconnector;

awe_uxconnector* awe_uxconnector_create(awe_uxscheduler_t* uxsched,
		connector_connectedhandler* connhandler, void* userdata);

void awe_uxconnector_destroy(awe_uxconnector *connector);

awe_status_t awe_uxconnector_open(awe_uxconnector* connector, awe_connection_t* udpconn,
		int type, const char* ip, uint16_t port, int32_t timeout, uint32_t id);
awe_status_t awe_uxconnector_openaddr(awe_uxconnector* connector, awe_connection_t* udpconn,
		int type, struct sockaddr *addr, socklen_t addrlen, int32_t timeout, uint32_t id);

awe_status_t awe_uxconnector_open2(awe_uxconnector* connector, awe_connection_t* uxconn);

void awe_uxconnector_close(awe_uxconnector* connector);

awe_connection_t* awe_uxconnector_takeout(awe_uxconnector *connector);

void awe_uxconnector_setEventHandler(awe_uxconnector *connector, int ufd,
		sockethandler_proc* proc, void* userdata);

void awe_uxconnector_rmEventHandler(awe_uxconnector *connector);

AWE_END_DECLS

#endif /* AWE_CONNECTOR_H_ */
