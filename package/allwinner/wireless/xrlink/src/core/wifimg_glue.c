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


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include "log_core.h"
#include "os_net_sync_notify.h"
#include "xr_proto.h"
#include "wifimg.h"
#include "xr_cmd.h"
#include "os_net_thread.h"
#include "api_action.h"
#include "wifimg_glue.h"

#define WAIT_DEV_NORMOL_TO_MS  (5000)
#define WAIT_DEV_CONNECT_TO_MS (15000)

#define U16_F "hu"

#define WMG_LOG_TAG "WFG"

#define WMG_INFO(fmt,arg...) \
	log_print(WMG_LOG_TAG,LOG_LEVEL_INFO,fmt,##arg)

#define WMG_DEBUG(fmt,arg...) \
	log_print(WMG_LOG_TAG,LOG_LEVEL_DEBUG,fmt,##arg)

#define WMG_WARNG(fmt,arg...) \
	log_print(WMG_LOG_TAG,LOG_LEVEL_WARNING,fmt,##arg)

#define WMG_ERROR(fmt,arg...) \
	log_print(WMG_LOG_TAG,LOG_LEVEL_ERROR,fmt,##arg)

#define RETURN_WMG_STATUS(status)                                   \
    do {                                                            \
        memcpy(cb_args[0], (void*)(&status), sizeof(wmg_status_t)); \
        return status;                                              \
    } while(0)

static os_net_thread_t wifimg_thread;
static snfy_handle_t *wifi_snfy_handle = NULL;
static wifi_msg_cb_t local_msg_cb = NULL;
static wifi_vend_cb_t vend_cb = NULL;

static char *country_code_table[] =
{
	"AU",	//Australia
	"CA",	//Canada
	"CN",	//China
	"DE",	//Deutschland
	"EU",	//European Union
	"FR",	//France
	"JP",	//Japan
	"RU",	//Russia;
	"SA",	//Saudi Arabia
	"US",	//US
	"NULL",
};

static int get_net_ip(const char *if_name, char *ip, int *len, int *vflag)
{
	struct ifaddrs *ifAddrStruct = NULL, *pifaddr = NULL;
	void *tmpAddrPtr = NULL;

	*vflag = 0;
	getifaddrs(&ifAddrStruct);
	pifaddr = ifAddrStruct;
	while (pifaddr != NULL) {
		if (NULL == pifaddr->ifa_addr) {
			pifaddr=pifaddr->ifa_next;
			continue;
		} else if (pifaddr->ifa_addr->sa_family == AF_INET) { // check it is IP4
			tmpAddrPtr = &((struct sockaddr_in *)pifaddr->ifa_addr)->sin_addr;
			if (strcmp(pifaddr->ifa_name, if_name) == 0) {
				inet_ntop(AF_INET, tmpAddrPtr, ip, INET_ADDRSTRLEN);
				*vflag = 4;
				break;
			}
		} else if (pifaddr->ifa_addr->sa_family == AF_INET6) { // check it is IP6
			// is a valid IP6 Address
			tmpAddrPtr = &((struct sockaddr_in *)pifaddr->ifa_addr)->sin_addr;
			if (strcmp(pifaddr->ifa_name, if_name) == 0) {
				inet_ntop(AF_INET6, tmpAddrPtr, ip, INET6_ADDRSTRLEN);
				*vflag=6;
				break;
			}
		}
		pifaddr=pifaddr->ifa_next;
	}

	if (ifAddrStruct != NULL) {
		freeifaddrs(ifAddrStruct);
	}

    return 0;
}

static void xr_wifi_config_ip_address(void *arg)
{
	char config_ip[64];
	char ip_addr[16];
	char netmask[16];
	char gw[16];

	int len = 0, vflag = 0;
	char ipaddr[INET6_ADDRSTRLEN];

	get_net_ip("wlan0", ipaddr, &len, &vflag);

	if(vflag) {
		WMG_DEBUG("ip address already config\n");
		return ;
	}

	xr_wifi_sta_ip_info_t *ip_info = (xr_wifi_sta_ip_info_t *)arg;

	sprintf(ip_addr, "%"U16_F".%"U16_F".%"U16_F".%"U16_F"%c",
			(unsigned short)ip_info->ip_addr[0], (unsigned short)ip_info->ip_addr[1],
			(unsigned short)ip_info->ip_addr[2], (unsigned short)ip_info->ip_addr[3], '\0');

	sprintf(netmask, "%"U16_F".%"U16_F".%"U16_F".%"U16_F"%c",
			(unsigned short)ip_info->netmask[0], (unsigned short)ip_info->netmask[1],
			(unsigned short)ip_info->netmask[2], (unsigned short)ip_info->netmask[3], '\0');

	sprintf(gw, "%"U16_F".%"U16_F".%"U16_F".%"U16_F"%c",
			(unsigned short)ip_info->gw[0], (unsigned short)ip_info->gw[1],
			(unsigned short)ip_info->gw[2], (unsigned short)ip_info->gw[3], '\0');

	WMG_DEBUG("ip address:%s\n", ip_addr);
	WMG_DEBUG("netmask:%s\n", netmask);
	WMG_DEBUG("gw:%s\n", gw);

	sprintf(config_ip,"ifconfig wlan0 %s netmask %s%c",
			ip_addr,netmask,'\0');
	WMG_DEBUG("config ip:%s\n", config_ip);
	system(config_ip);

	sprintf(config_ip,"route add default gw %s%c",gw,'\0');

	WMG_DEBUG("config gw:%s\n", config_ip);
	system(config_ip);

	sprintf(config_ip,"echo nameserver %s > /etc/resolv.conf%c",gw,'\0');
	WMG_DEBUG("config dns:%s\n", config_ip);
	system(config_ip);

}

static void xr_wifi_msg_cb(char *data,uint32_t len)
{
	xr_cfg_payload_t *payload = (xr_cfg_payload_t*) data;
	xr_wifi_sta_cn_event_t *cn_event;
	xr_wifi_raw_data_t *raw_data;
	xr_wifi_dev_status_t *d_status;

	wifi_msg_data_t msg = {
		.id = WIFI_MSG_ID_MAX,
	};

	WMG_DEBUG("dev ind msg type:%d\n", payload->type);

	switch(payload->type) {
		case XR_WIFI_DEV_STA_CN_EV:

			cn_event = (xr_wifi_sta_cn_event_t *) payload->param;
			WMG_DEBUG("wifi sta connect event:%d\n", cn_event->event);
			if(cn_event->event == XR_WIFI_DHCP_SUCCESS) {
				snfy_ready(wifi_snfy_handle, payload->param);
			}

			msg.id = WIFI_MSG_ID_STA_CN_EVENT;
			msg.data.event = cn_event->event;

			break;

		case XR_WIFI_DEV_DEV_STATUS:

			d_status = (xr_wifi_dev_status_t *)payload->param;

			msg.id = WIFI_MSG_ID_DEV_STATUS;
			msg.data.d_status = d_status->status;

		case XR_WIFI_DEV_STA_INFO:
		case XR_WIFI_DEV_SCAN_RES:
		case XR_WIFI_DEV_MAC:
		case XR_WIFI_DEV_AP_STATE:
		case XR_WIFI_DEV_AP_INF:
		case XR_WIFI_DEV_NETWORKS:
		case XR_WIFI_DEV_COUNTRY_CODE:
			snfy_ready(wifi_snfy_handle, payload->param);
			break;
		case XR_WIFI_DEV_IND_IP_INFO:
			xr_wifi_config_ip_address(payload->param);
			break;
		case XR_WIFI_DEV_IND_RAW_DATA:
			raw_data = (xr_wifi_raw_data_t *) payload->param;
			if(vend_cb) {
				vend_cb(raw_data->data, raw_data->len);
			}
			break;
		default:
			WMG_WARNG("unkown payload msg type (%d)\n", payload->type);
			break;
	}

	if(msg.id != WIFI_MSG_ID_MAX && local_msg_cb)
		local_msg_cb(&msg);

	if(msg.id == WIFI_MSG_ID_STA_CN_EVENT) {

		switch(msg.data.event) {
			case XR_WIFI_ASSOCIATING:
				msg.id = WIFI_MSG_ID_STA_STATE_CHANGE;
				msg.data.state = WIFI_STA_CONNECTING;
				break;
			case XR_WIFI_CONNECTED:
				msg.id = WIFI_MSG_ID_STA_STATE_CHANGE;
				msg.data.state = WIFI_STA_CONNECTED;
				break;
			case XR_WIFI_DHCP_SUCCESS:
				msg.id = WIFI_MSG_ID_STA_STATE_CHANGE;
				msg.data.state = WIFI_STA_NET_CONNECTED;
				break;
			case XR_WIFI_DISCONNECTED:
				msg.id = WIFI_MSG_ID_STA_STATE_CHANGE;
				msg.data.state = WIFI_STA_DISCONNECTED;
				break;
			default:
				break;
		}

		if(msg.id == WIFI_MSG_ID_STA_STATE_CHANGE && local_msg_cb) {
			local_msg_cb(&msg);
		}
	}
}

static int wifi_glue_register_msg_cb(void **call_args, void **cb_args)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;

	wifi_msg_cb_t msg_cb = (wifi_msg_cb_t) call_args[0];

	local_msg_cb = msg_cb;

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_on(void **call_args, void **cb_args)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	xr_wifi_dev_status_t *dev_status;

	wifi_mode_t wf_mode = (wifi_mode_t) call_args[0];

	if(!wifi_snfy_handle) {
		wifi_snfy_handle = snfy_new();
	}

	xr_wifi_register_msg_cb(xr_wifi_msg_cb);

	xr_wifi_init();

	status = xr_wifi_on((xr_wifi_mode_t)wf_mode);

	dev_status = (xr_wifi_dev_status_t *)snfy_await(wifi_snfy_handle,
			WAIT_DEV_NORMOL_TO_MS);

	if(dev_status)
		WMG_DEBUG("wifi dev status: %s\n", dev_status->status ?
					"WLAN UP": "WLAN DOWN");

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_off(void **call_args, void **cb_args)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	xr_wifi_dev_status_t *dev_status;

	status = xr_wifi_off();

	dev_status = (xr_wifi_dev_status_t *)snfy_await(wifi_snfy_handle,
			WAIT_DEV_NORMOL_TO_MS);

	xr_wifi_deinit();

	if(dev_status)
		WMG_DEBUG("wifi dev status: %s\n", dev_status->status ?
					"WLAN UP": "WLAN DOWN");

	if(wifi_snfy_handle) {
		snfy_free(wifi_snfy_handle);
		wifi_snfy_handle = NULL;
	}

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_sta_connect(void **call_args, void **cb_args)
{
	wmg_status_t status = OS_NET_STATUS_FAILED;

	wifi_sta_cn_para_t *cn = (wifi_sta_cn_para_t *) call_args[0];

	xr_wifi_sta_cn_t *con = malloc(sizeof(xr_wifi_sta_cn_t));

	memset(con, 0, sizeof(xr_wifi_sta_cn_t));
	con->sec = cn->sec;
	con->fast_connect = cn->fast_connect;
	memcpy(con->ssid, cn->ssid, strlen(cn->ssid));
	memcpy(con->pwd, cn->password, strlen(cn->password));

	WMG_DEBUG("[wifi_sta_connect] cn->ssid len=%d, %s\n",
			strlen(cn->ssid), cn->ssid);
	WMG_DEBUG("[wifi_sta_connect] cn->password len=%d, %s\n",
			strlen(cn->password), cn->password);

	status = xr_wifi_sta_connect(con);

	if(snfy_await(wifi_snfy_handle, WAIT_DEV_CONNECT_TO_MS) == NULL) {
		goto failed;
	}

	free(con);
	RETURN_WMG_STATUS(status);

failed:
	status = OS_NET_STATUS_FAILED;
	free(con);
	RETURN_WMG_STATUS(status);
}

static int wifi_glue_sta_disconnect(void **call_args, void **cb_args)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;

	status = xr_wifi_sta_disconnect();

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_sta_auto_reconnect(void **call_args, void **cb_args)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	wmg_bool_t enable = (wmg_bool_t) call_args[0];

	status = xr_wifi_sta_auto_reconnect(enable);

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_sta_get_info(void **call_args, void **cb_args)
{
	wifi_sta_info_t *sta_info = (wifi_sta_info_t *) call_args[0];
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	xr_wifi_sta_info_t *info;

	status = xr_wifi_sta_get_info(NULL);
	info = (xr_wifi_sta_info_t *)snfy_await(wifi_snfy_handle,
					WAIT_DEV_NORMOL_TO_MS);
	if (info) {
		sta_info->id	= info->id;
		sta_info->freq	= info->freq;
		sta_info->sec	= info->sec;
		memcpy(sta_info->bssid, info->bssid, 6);
		memcpy(sta_info->ssid, info->ssid, strlen(info->ssid));
		memcpy(sta_info->mac_addr, info->mac_addr, 6);
		memcpy(sta_info->ip_addr, info->ip_addr, 4);
	} else {
		status = OS_NET_STATUS_FAILED;
	}

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_sta_list_networks(void **call_args, void **cb_args)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	WMG_DEBUG("list network not support\n");
	RETURN_WMG_STATUS(status);
}

static int wifi_glue_sta_remove_networks(void **call_args, void **cb_args)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	WMG_DEBUG("remove network not support\n");
	RETURN_WMG_STATUS(status);
}

static int wifi_glue_ap_enable(void **call_args, void **cb_args)
{
	wifi_ap_config_t *ap_config = (wifi_ap_config_t *) call_args[0];
	wmg_status_t status = WMG_STATUS_UNHANDLED;

	xr_wifi_ap_config_t *cfg = (xr_wifi_ap_config_t *)malloc(sizeof(xr_wifi_ap_config_t));
	memset(cfg, 0, sizeof(xr_wifi_ap_config_t));

	cfg->channel = ap_config->channel;
	cfg->sec = ap_config->sec;

	memcpy(cfg->ssid, ap_config->ssid, strlen(ap_config->ssid));
	if (cfg->sec) {
		memcpy(cfg->pwd, ap_config->psk, strlen(ap_config->psk));
	}
	status = xr_wifi_ap_enable(cfg);

	xr_wifi_ap_state_t *ap_state = (xr_wifi_ap_state_t *)snfy_await(wifi_snfy_handle,
					WAIT_DEV_NORMOL_TO_MS);
	if(ap_state) {
		WMG_DEBUG("enable ap state:%d\n", ap_state->state);
	} else
		status = OS_NET_STATUS_FAILED;

	free(cfg);

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_ap_disable(void **call_args, void **cb_args)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;

	status = xr_wifi_ap_disable();

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_ap_get_config(void **call_args, void **cb_args)
{
	wifi_ap_config_t *ap_config = (wifi_ap_config_t *) call_args[0];
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	int i;
	char mac[18 + 1] = {0};
	xr_wifi_ap_sta_info_t *ap_sta_info;

	status = xr_wifi_ap_get_config_info(NULL);

	xr_wifi_ap_info_t *info = (xr_wifi_ap_info_t *)snfy_await(wifi_snfy_handle,
					WAIT_DEV_NORMOL_TO_MS);
	if(info) {
		ap_config->channel = info->channel;
		ap_config->sta_num = info->sta_num;
		WMG_DEBUG("sta number:%d\n", info->sta_num);
		ap_config->sec = info->sec;
		memcpy(ap_config->ssid, info->ssid, strlen(info->ssid));
		memcpy(ap_config->psk, info->pwd, strlen(info->pwd));
		ap_sta_info = (xr_wifi_ap_sta_info_t *) info->dev_list;
		WMG_DEBUG("sta number:%d\n", info->sta_num);
		for(i = 0; i < info->sta_num; i++) {
			sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X%c",
				ap_sta_info[i].addr[0],ap_sta_info[i].addr[1],ap_sta_info[i].addr[2],
				ap_sta_info[i].addr[3],ap_sta_info[i].addr[4],ap_sta_info[i].addr[5],'\0');
			WMG_DEBUG("mac addres:%s\n", mac);
			memcpy(ap_config->dev_list[i], mac, 18);
		}
	} else
		status = OS_NET_STATUS_FAILED;

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_monitor_enable(void **call_args, void **cb_args)
{
	uint8_t channel = (uint8_t) call_args[0];
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	xr_wifi_monitor_enable_t mon_arg;

	mon_arg.channel = channel;
	status = xr_wifi_monitor_enable(&mon_arg);

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_monitor_set_channel(void **call_args, void **cb_args)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	uint8_t channel = (uint8_t) call_args[0];

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_monitor_disable(void **call_args, void **cb_args)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;

	status = xr_wifi_monitor_disable();

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_set_scan_param(void **call_args, void **cb_args)
{
	wifi_scan_param_t *scan_param = (wifi_scan_param_t *) call_args[0];
	wmg_status_t status = WMG_STATUS_UNHANDLED;

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_get_scan_results(void **call_args, void **cb_args)
{
	wifi_scan_result_t *result = (wifi_scan_result_t *) call_args[0];
	uint32_t *bss_num = (uint32_t *) call_args[1];
	uint32_t arr_size = (uint32_t) call_args[2];

	wmg_status_t status = WMG_STATUS_UNHANDLED;
	xr_wifi_scan_result_t *scan_result = NULL;
	xr_wifi_scan_info_t *scan_info = NULL;
	int i;
	char bssid[18 + 1];

	status = xr_wifi_get_scan_results();
	scan_result = (xr_wifi_scan_result_t *)snfy_await(wifi_snfy_handle,
							WAIT_DEV_NORMOL_TO_MS);

	if (scan_result) {
		scan_info = (xr_wifi_scan_info_t *)scan_result->ap_info;
		*bss_num = scan_result->num;
		for (i = 0; i < *bss_num; i++) {
			result[i].freq = scan_info[i].freq;
			result[i].rssi = scan_info[i].rssi;
			result[i].key_mgmt = scan_info[i].key_mgmt;

			sprintf(bssid,"%02X:%02X:%02X:%02X:%02X:%02X%c",
				scan_info[i].bssid[0], scan_info[i].bssid[1], scan_info[i].bssid[2],
				scan_info[i].bssid[3], scan_info[i].bssid[4], scan_info[i].bssid[5], '\0');

			memcpy(result[i].bssid, bssid, 18);
			memcpy(result[i].ssid, scan_info[i].ssid, strlen(scan_info[i].ssid));
		}
	} else {
		status = OS_NET_STATUS_FAILED;
	}
	RETURN_WMG_STATUS(status);
}

static int wifi_glue_set_mac(void **call_args, void **cb_args)
{
	const char *ifname = (const char *) call_args[0];
	uint8_t *mac_addr = (uint8_t *) call_args[1];

	wmg_status_t status = WMG_STATUS_UNHANDLED;
	xr_wifi_mac_info_t *mi = (xr_wifi_mac_info_t *)malloc(sizeof(xr_wifi_mac_info_t));

	memset(mi, 0, sizeof(xr_wifi_mac_info_t));
	memcpy(mi->mac, mac_addr, 6);
	memcpy(mi->ifname, ifname, 5);
	status = xr_wifi_set_mac(mi);
	free(mi);
	RETURN_WMG_STATUS(status);
}

static int wifi_glue_get_mac(void **call_args, void **cb_args)
{
	const char *ifname = (const char *) call_args[0];
	uint8_t *mac_addr = (uint8_t *) call_args[1];

	wmg_status_t status = WMG_STATUS_UNHANDLED;
	xr_wifi_mac_info_t *mac;

	status = xr_wifi_get_mac(NULL);
	mac = (xr_wifi_mac_info_t *)snfy_await(wifi_snfy_handle, WAIT_DEV_NORMOL_TO_MS);
	if (mac) {
		memcpy(mac_addr, mac->mac, 6);
		WMG_DEBUG("[wifi_get_mac] get result ifname=%s\n", ifname);
	} else {
		status = OS_NET_STATUS_FAILED;
	}

	WMG_DEBUG("[wifi_get_mac] get result status=%d\n", status);
	RETURN_WMG_STATUS(status);
}

static int wifi_glue_send_80211_rframe(void **call_args, void **cb_args)
{
	uint8_t *data = (uint8_t *) call_args[0];
	uint32_t len = (uint32_t) call_args[1];
	void *priv = (void *) call_args[2];

	wmg_status_t status = WMG_STATUS_UNHANDLED;

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_set_country_code(void **call_args, void **cb_args)
{
	const char *country_code = (const char *) call_args[0];
	int i;
	wmg_status_t status = WMG_STATUS_UNHANDLED;

	xr_wifi_country_code_info_t country;
	for (i=0; i<sizeof(country_code_table)/sizeof(country_code_table[0]); i++) {
		if (!memcmp(country_code_table[i], country_code, 2)) {
			country.code = i;
			break;
		}
	}

	status = xr_wifi_set_country_code(&country);

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_get_country_code(void **call_args, void **cb_args)
{
	char *country_code = (char *) call_args[0];

	wmg_status_t status = WMG_STATUS_UNHANDLED;
	xr_wifi_country_code_info_t *country;

	status = xr_wifi_get_country_code();
	country = (xr_wifi_country_code_info_t *)snfy_await(wifi_snfy_handle,
							WAIT_DEV_NORMOL_TO_MS);
	if (country) {
		memcpy(country_code, country_code_table[country->code], 2);
	} else {
		status = OS_NET_STATUS_FAILED;
	}
	RETURN_WMG_STATUS(status);
}

static int wifi_glue_set_ps_mode(void **call_args, void **cb_args)
{
	wmg_bool_t enable = (wmg_bool_t) call_args[0];

	wmg_status_t status = WMG_STATUS_UNHANDLED;

	RETURN_WMG_STATUS(status);
}

static int wifi_glue_vendor_register_cb(void **call_args, void **cb_args)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	vend_cb = (wifi_vend_cb_t) call_args[0];
	if(vend_cb) {
		status = WMG_STATUS_SUCCESS;
	} else {
		status = WMG_STATUS_INVALID;
	}
	RETURN_WMG_STATUS(status);
}

static int wifi_glue_vendor_send_data(void **call_args, void **cb_args)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	uint8_t *data = (uint8_t *) call_args[0];
	uint32_t len = (uint32_t) call_args[1];

	xr_wifi_raw_data_t *raw = (xr_wifi_raw_data_t *)malloc(sizeof(xr_wifi_raw_data_t) + len);
	if(NULL == raw) {
		status = WMG_STATUS_NOMEM;
		goto end;
	}

	raw->len = len;

	memcpy(raw->data, data, len);

	status = xr_wifi_send_raw_data(raw);

	free(raw);
end:
	RETURN_WMG_STATUS(status);
}

act_func_t wifimg_action_table[] = {
    [ACWIFI_REG_CB]             = { wifi_glue_register_msg_cb,     "reg_cb" },
    [ACWIFI_ON]                 = { wifi_glue_on,                  "on" },
    [ACWIFI_OFF]                = { wifi_glue_off,                 "off" },
    [ACWIFI_STA_CONNECT]        = { wifi_glue_sta_connect,         "sta_connect" },
    [ACWIFI_STA_DISCONNECT]     = { wifi_glue_sta_disconnect,      "sta_disconnect" },
    [ACWIFI_AUTO_RECONNECT]     = { wifi_glue_sta_auto_reconnect,  "auto_reconnect" },
    [ACWIFI_STA_GET_INFO]       = { wifi_glue_sta_get_info,        "sta_get_info" },
    [ACWIFI_LIST_NETWORKS]      = { wifi_glue_sta_list_networks,   "list_networks" },
    [ACWIFI_REMOVE_NETWORKS]    = { wifi_glue_sta_remove_networks, "remove_networks" },
    [ACWIFI_AP_ENABLE]          = { wifi_glue_ap_enable,           "ap_enable" },
    [ACWIFI_AP_DISABLE]         = { wifi_glue_ap_disable,          "ap_disable" },
    [ACWIFI_AP_GET_CONFIG]      = { wifi_glue_ap_get_config,       "ap_get_config" },
    [ACWIFI_MON_ENABLE]         = { wifi_glue_monitor_enable,      "mon_enable"},
    [ACWIFI_MON_SET_CH]         = { wifi_glue_monitor_set_channel, "mon_set_channel" },
    [ACWIFI_MON_DISABLE]        = { wifi_glue_monitor_disable,     "mon_disable" },
    [ACWIFI_SET_SCAN_PARAM] = { wifi_glue_set_scan_param,      "mon_set_scan_param" },
    [ACWIFI_GET_SCAN_RESULTS]   = { wifi_glue_get_scan_results,    "get_scan_results" },
    [ACWIFI_SET_MAC]            = { wifi_glue_set_mac,             "set_mac" },
    [ACWIFI_GET_MAC]            = { wifi_glue_get_mac,             "get_mac" },
    [ACWIFI_SEND_80211_FRAME]   = { wifi_glue_send_80211_rframe,    "send_80211_frame" },
    [ACWIFI_SET_COUNTRY_CODE]   = { wifi_glue_set_country_code,    "set_country_code" },
    [ACWIFI_GET_COUNTRY_CODE]   = { wifi_glue_get_country_code,    "get_country_code" },
    [ACWIFI_SET_PS_MODE]        = { wifi_glue_set_ps_mode,         "set_ps_mode" },
    [ACWIFI_VENDOR_SEND_DATA]   = { wifi_glue_vendor_send_data,    "vendor_send" },
    [ACWIFI_VENDOR_REG_CB]      = { wifi_glue_vendor_register_cb,  "vendor_reg" }
};

