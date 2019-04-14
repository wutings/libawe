/*
 * connector.c
 *
 * Copyright (c) 2016-2019 Tim <and-joy@qq.com>
 * All rights reserved.
 */

#include <ux.h>

#define AWE_LOG_TAG "connector"
#include "awe/log.h"

#include "awe/connector.h"

static void connector_connecthandling(void* userdata, int fd, int mask){
	awe_connector *connector = (awe_connector *)userdata;
	awe_scheduler_socktoken_destroy(connector->_sched, &connector->_conntoken);
	awe_scheduler_tasktoken_destroy(connector->_sched, &connector->_timeouttoken);

	int err;
	int rs = awe_socket_connected(fd, &err);
	connector->_onconnected_proc(connector->_userdata, rs, fd);
}

static void connector_connecttimeout(void* userdata, long arg){
	awe_connector *connector = (awe_connector *)userdata;

	awe_connector_close(connector);
	connector->_onconnected_proc(connector->_userdata, AWE_CONN_TIMEDOUT, 0);
}

awe_connector* awe_connector_create(awe_scheduler_t* sched,
		connector_connectedhandler* connhandler, void* userdata){
	awe_connector *connector = awe_mallocz(sizeof(awe_connector));
	connector->_sched = sched;
	connector->_conn = NULL;
	connector->_conntoken = NULL;
	connector->_timeouttoken = NULL;
	connector->_onconnected_proc = connhandler;
	connector->_userdata = userdata;
	return connector;
}

void awe_connector_destroy(awe_connector *connector){
	awe_connector_close(connector);

	awe_connection_autorelease(&connector->_conn);

	awe_free(connector);
}

awe_status_t awe_connector_open(awe_connector *connector, const char* ip, uint16_t port,
		int32_t timeout, uint32_t id){
	ALOG_ASSERT(connector->_conn != NULL);

	connector->_conn = awe_connection_stream2(AF_UNSPEC, id);

	awe_status_t rs = awe_connection_open(connector->_conn, ip, port, 0);
	if(rs == 0 || rs == 1){//0:connecting, 1:connected
		if(timeout <= 0 || timeout >= 60000){
			timeout = 60000;
		}
		connector->_timeouttoken = awe_scheduler_delayedtask(connector->_sched,
				timeout, connector_connecttimeout, connector, 0);

		int fd = awe_connection_fd(connector->_conn);
		connector->_conntoken = awe_scheduler_watch(connector->_sched,
				fd, AWE_WRITE, connector_connecthandling, connector);
		return AWE_OK;
	}else{//error
		awe_connector_close(connector);
		return -1;
	}
}

awe_status_t awe_connector_open2(awe_connector *connector, awe_connection_t* tcpconn){
	ALOG_ASSERT(connector->_conn != NULL);
	connector->_conn = awe_connection_addref(tcpconn);
	return AWE_OK;
}

void awe_connector_close(awe_connector *connector){
	awe_connector_rmEventHandler(connector);

	if(connector->_conn != NULL){
		awe_connection_close(connector->_conn);
	}
}

awe_connection_t* awe_connector_takeout(awe_connector *connector){
	awe_connector_rmEventHandler(connector);

	awe_connection_t* temp = connector->_conn;
	connector->_conn = NULL;
	return temp;
}

void awe_connector_setEventHandler(awe_connector *connector,
		int fd, sockethandler_proc* proc, void* userdata){
	if(connector->_conntoken == NULL){
		connector->_conntoken = awe_scheduler_watch(connector->_sched,
				fd, AWE_READ, proc, userdata);
	}
}

void awe_connector_rmEventHandler(awe_connector *connector){
	if(connector->_sched != NULL){
		awe_scheduler_socktoken_destroy(connector->_sched, &connector->_conntoken);
		awe_scheduler_tasktoken_destroy(connector->_sched, &connector->_timeouttoken);
	}
}

//===============================================================================
static void uxconnector_connecthandling(void* userdata, int ufd, int mask){
	awe_uxconnector *connector = (awe_uxconnector *)userdata;
	ALOG_ASSERT(awe_uxscheduler_cancel(connector->_uxsched, connector->_timeouttask_id) != AWE_OK);

	awe_uxscheduler_rm(connector->_uxsched, ufd);

	int rs = ux_connected(ufd);
	connector->_onconnected_proc(connector->_userdata, rs == AWE_OK ? AWE_OK : AWE_CONN_TIMEDOUT, ufd);
}

static void uxconnector_connecttimeout(void* userdata, long arg){
	awe_uxconnector *connector = (awe_uxconnector *)userdata;

	awe_uxconnector_close(connector);

	connector->_onconnected_proc(connector->_userdata, AWE_CONN_TIMEDOUT, 0);
}

awe_uxconnector* awe_uxconnector_create(awe_uxscheduler_t* uxsched,
		connector_connectedhandler* connhandler, void* userdata){
	awe_uxconnector* connector = awe_mallocz(sizeof(awe_uxconnector));
	connector->_uxsched = uxsched;
	connector->_uxconn = NULL;

	connector->_onconnected_proc = connhandler;
	connector->_userdata = userdata;
	return connector;
}

void awe_uxconnector_destroy(awe_uxconnector *connector){
	awe_uxconnector_close(connector);

	awe_connection_autorelease(&connector->_uxconn);

	awe_free(connector);
}

static void uxconnector_setTimeout(awe_uxconnector* connector, int32_t timeout){
	if(timeout <= 0 || timeout >= 60000){
		timeout = 60000;
	}
	connector->_timeouttask_id = awe_uxscheduler_post(connector->_uxsched,
			timeout, uxconnector_connecttimeout, connector, 0);

	int ufd = awe_connection_fd(connector->_uxconn);
	awe_uxscheduler_add(connector->_uxsched, ufd, AWE_WRITE,
			uxconnector_connecthandling, connector);
}

awe_status_t awe_uxconnector_open(awe_uxconnector* connector, awe_connection_t* udpconn,
		int type, const char* ip, uint16_t port, int32_t timeout, uint32_t id){
	ALOG_ASSERT(connector->_uxconn != NULL);

	awe_status_t rs = -1;
	connector->_uxconn = awe_connection_ux2(type, udpconn, id);
	rs = awe_connection_open(connector->_uxconn, ip, port, 0);
	if(rs == 0){
		uxconnector_setTimeout(connector, timeout);
		return AWE_OK;
	}else{
		awe_uxconnector_close(connector);
		return -1;
	}
}

awe_status_t awe_uxconnector_openaddr(awe_uxconnector* connector, awe_connection_t* udpconn,
		int type, struct sockaddr *addr, socklen_t addrlen, int32_t timeout, uint32_t id){
	ALOG_ASSERT(connector->_uxconn != NULL);

	awe_status_t rs = -1;
	connector->_uxconn = awe_connection_ux2(type, udpconn, id);
	rs = awe_connection_open2(connector->_uxconn, addr, addrlen, 0);
	if(rs == 0){
		uxconnector_setTimeout(connector, timeout);
		return AWE_OK;
	}else{
		awe_uxconnector_close(connector);
		return -1;
	}
}

awe_status_t awe_uxconnector_open2(awe_uxconnector* connector, awe_connection_t* uxconn){
	ALOG_ASSERT(connector->_uxconn != NULL);

	connector->_uxconn = awe_connection_addref(uxconn);
	return AWE_OK;
}

void awe_uxconnector_close(awe_uxconnector* connector){
	awe_uxconnector_rmEventHandler(connector);

	if(connector->_uxconn != NULL){
		awe_connection_close(connector->_uxconn);
	}
}

awe_connection_t* awe_uxconnector_takeout(awe_uxconnector *connector){
	awe_uxconnector_rmEventHandler(connector);

	awe_connection_t* temp = connector->_uxconn;
	connector->_uxconn = NULL;

	return temp;
}

void awe_uxconnector_setEventHandler(awe_uxconnector *connector, int ufd,
		sockethandler_proc* proc, void* userdata){
	awe_uxscheduler_add(connector->_uxsched, ufd, AWE_READ, proc, userdata);
}

void awe_uxconnector_rmEventHandler(awe_uxconnector *connector){
	if(connector->_uxsched != NULL && connector->_uxconn != NULL){
		awe_uxscheduler_rm(connector->_uxsched, awe_connection_fd(connector->_uxconn));
		awe_uxscheduler_cancel(connector->_uxsched, connector->_timeouttask_id);
	}
}
