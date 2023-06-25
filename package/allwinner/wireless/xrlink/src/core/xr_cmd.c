/*
 * Copyright (C) 2022 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <string.h>
#include <stdint.h>
#include "os_net_utils.h"
#include "xr_cmd.h"
#include "msg_netlink.h"


static nlmsg_priv_t *xr_nlmsg = NULL;
static xr_wifi_msg_cb_t cmd_cb = NULL;

#define DATA_LEN_ALIGNMENT 4

#define XRNL_MSG_STATE_CK()                         \
    do {                                            \
		if(NULL == xr_nlmsg) {                      \
			CMD_ERROR("xrnl msg is not ready\n");   \
			return OS_NET_STATUS_NOT_READY;         \
		}                                           \
	} while(0)


static xr_cfg_payload_t *xr_wifi_cfg_payload_calloc(uint32_t len,uint16_t type,void *param)
{
	xr_cfg_payload_t *cfg_payload = NULL;
	int tot_len;
	tot_len = CMD_HEAD_LEN + len;
	tot_len +=DATA_LEN_ALIGNMENT - tot_len%DATA_LEN_ALIGNMENT;

	CMD_DEBUG("header len:%d,payload len:%d,tot_len:%d\n",CMD_HEAD_LEN,len,tot_len);
	cfg_payload = (xr_cfg_payload_t *) malloc(tot_len);

	cfg_payload->type = type;
	cfg_payload->len = len;
	if(param)
		memcpy(cfg_payload->param,param,len);
	return cfg_payload;
}

static void xr_wifi_cfg_payload_free(xr_cfg_payload_t *cfg_payload)
{
	if(cfg_payload) {
		free(cfg_payload);
	}
}

static int xr_wifi_recv_cb(char *data,uint32_t len)
{
	//log_hex_dump(__func__,20,data,len);

	if(cmd_cb) {
		cmd_cb(data, len);
	}
	return 0;
}

os_net_status_t xr_wifi_init(void)
{
	if(!xr_nlmsg) {
		xr_nlmsg = nlmsg_init(xr_wifi_recv_cb);
		if(!xr_nlmsg) {
			return OS_NET_STATUS_FAILED;
		}
	}
	return OS_NET_STATUS_OK;
}

os_net_status_t xr_wifi_deinit(void)
{
	if(xr_nlmsg) {
		CMD_DEBUG("[xr_wifi_off] nlmsg_deinit\r\n");
		nlmsg_deinit(xr_nlmsg);
		xr_nlmsg = NULL;
	}
	return OS_NET_STATUS_OK;
}

os_net_status_t xr_wifi_on(xr_wifi_mode_t mode)
{
	os_net_status_t status;
	uint8_t cfg_mode;
	cfg_mode = mode;
	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(sizeof(xr_wifi_on_t),
			XR_WIFI_HOST_ON,&cfg_mode);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}

	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}

	xr_wifi_cfg_payload_free(cfg_payload);
	return status;
}


os_net_status_t xr_wifi_off(void)
{
	os_net_status_t status;
	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(0,
			XR_WIFI_HOST_OFF,NULL);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}

	xr_wifi_cfg_payload_free(cfg_payload);

	return status;
}

os_net_status_t xr_wifi_sta_connect(xr_wifi_sta_cn_t *cn)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(sizeof(xr_wifi_sta_cn_t),
			XR_WIFI_HOST_STA_CONNECT,cn);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);
	return status;
}

os_net_status_t xr_wifi_sta_disconnect(void)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(0,
			XR_WIFI_HOST_STA_DISCONNECT,NULL);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);
	return status;
}

os_net_status_t xr_wifi_sta_auto_reconnect(xr_bool_t en)
{
	os_net_status_t status;
	uint8_t cfg_mode = en;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(sizeof(xr_sta_auto_reconnect_t),
			XR_WIFI_HOST_STA_AUTO_RECONNECT,&cfg_mode);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);
	return status;
}

os_net_status_t xr_wifi_sta_get_info(xr_wifi_sta_info_t *sta_info)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(0,
			XR_WIFI_HOST_STA_GET_INFO,NULL);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);

	return status;
}

os_net_status_t xr_wifi_ap_enable(xr_wifi_ap_config_t *ap_config)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(sizeof(xr_wifi_ap_config_t),
			XR_WIFI_HOST_AP_ENABLE,ap_config);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);
	return status;
}

os_net_status_t xr_wifi_ap_disable(void)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(0,
			XR_WIFI_HOST_AP_DISABLE,NULL);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);
	return status;
}

os_net_status_t xr_wifi_ap_get_config_info(xr_wifi_ap_info_t *info)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(0,
			XR_WIFI_HOST_AP_GET_INF,NULL);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);

	//TODO: wait get config info
	return status;
}

os_net_status_t xr_wifi_monitor_enable(xr_wifi_monitor_enable_t *m)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(sizeof(xr_wifi_monitor_enable_t),
			XR_WIFI_HOST_MONITOR_ENABLE,NULL);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);
	return status;
}

os_net_status_t xr_wifi_monitor_disable(void)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(0,
			XR_WIFI_HOST_MONITOR_DISABLE,NULL);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);

	return status;
}

os_net_status_t xr_wifi_register_msg_cb(xr_wifi_msg_cb_t cb)
{
	cmd_cb = cb;
}

os_net_status_t xr_wifi_get_scan_results(void)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(0,
			XR_WIFI_HOST_GET_SCAN_RES,NULL);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);
	return status;
}

os_net_status_t xr_wifi_set_mac(xr_wifi_mac_info_t *mi)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(sizeof(xr_wifi_mac_info_t),
			XR_WIFI_HOST_SET_MAC,mi);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);

	return status;
}

os_net_status_t xr_wifi_get_mac(xr_wifi_mac_info_t *mi)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(0,
			XR_WIFI_HOST_GET_MAC,NULL);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}

	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload, cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);

	return status;
}

os_net_status_t xr_wifi_send_80211_frame(xr_wifi_send_raw_data_t *raw)
{

	XRNL_MSG_STATE_CK();

}

os_net_status_t xr_wifi_sta_list_networks(void)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(0,
			XR_WIFI_HOST_STA_LIST_NETWORKS,NULL);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);

	return status;
}

os_net_status_t xr_wifi_sta_remove_networks(char *data)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	if (!data) {
		return OS_NET_STATUS_PARAM_INVALID;
	}
	int len = strlen(data);

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(len,
			XR_WIFI_HOST_STA_REMOVE_NETWORKS,data);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);

	return status;
}

os_net_status_t xr_wifi_set_country_code(xr_wifi_country_code_info_t *code)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(sizeof(xr_wifi_country_code_info_t),
			XR_WIFI_HOST_SET_COUNTRY_CODE, code);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}
	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload,cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);

	return status;
}

os_net_status_t xr_wifi_get_country_code(void)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(0,
			XR_WIFI_HOST_GET_COUNTRY_CODE,NULL);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}

	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload, cfg_payload->len + sizeof(xr_cfg_payload_t));
	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);

	return status;
}

os_net_status_t xr_wifi_send_raw_data(xr_wifi_raw_data_t *data)
{
	os_net_status_t status;

	XRNL_MSG_STATE_CK();

	xr_cfg_payload_t *cfg_payload = xr_wifi_cfg_payload_calloc(sizeof(xr_wifi_raw_data_t) + data->len,
			XR_WIFI_HOST_SEND_RAW, data);
	if(!cfg_payload) {
		return OS_NET_STATUS_NOMEM;
	}

	status = nlmsg_send(xr_nlmsg,(char*)cfg_payload, cfg_payload->len +
			sizeof(xr_wifi_raw_data_t) + data->len);

	if(status != OS_NET_STATUS_OK) {
		CMD_ERROR("nlmsg send failed\n");
	}
	xr_wifi_cfg_payload_free(cfg_payload);

	return status;
}
