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
#include "log_core.h"
#include "wifimg.h"
#include "os_net_utils.h"
#include "os_net_thread.h"
#include "api_action.h"
#include "wifimg_glue.h"

#define ACT_ID_WMG 0

#define WMG_LOG_TAG "WFG"

#define WMG_INFO(fmt,arg...) \
	log_print(WMG_LOG_TAG,LOG_LEVEL_INFO,fmt,##arg)

#define WMG_DEBUG(fmt,arg...) \
	log_print(WMG_LOG_TAG,LOG_LEVEL_DEBUG,fmt,##arg)

#define WMG_WARNG(fmt,arg...) \
	log_print(WMG_LOG_TAG,LOG_LEVEL_WARNING,fmt,##arg)

#define WMG_ERROR(fmt,arg...) \
	log_print(WMG_LOG_TAG,LOG_LEVEL_ERROR,fmt,##arg)

static act_handle_t wmg_act_handle;

static uint8_t wifi_on_check =0;

#define WIFI_STATE_CHECK(status)                            \
    do {                                                    \
		if(wifi_on_check == 0) {                            \
			WMG_WARNG("WiFi device is not turned on,"       \
				"Now sta mode is turned on by default.\n"); \
			wifi_on(WIFI_STATION);                          \
		}                                                 \
	} while(0)

static void wifi_api_action_init(void)
{
	if(wmg_act_handle.enable)
		return;

    act_init(&wmg_act_handle, "wmg_action");

    act_register_handler(&wmg_act_handle, ACT_ID_WMG, wifimg_action_table);
}

static void wifi_api_action_deinit(void)
{
	if(!wmg_act_handle.enable)
		return;

    act_deinit(&wmg_act_handle);
}

wmg_status_t wifi_register_msg_cb(wifi_msg_cb_t msg_cb)
{
	wmg_status_t status = WMG_STATUS_FAIL;

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_REG_CB, 1, 1, msg_cb, &status);

	return status;
}

wmg_status_t wifi_on(wifi_mode_t mode)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WMG_INFO("wifimanager version:%s\n", WMG_VERSION);

	wifi_api_action_init();

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_ON, 1, 1, mode, &status);

	if(status != WMG_STATUS_SUCCESS) {
		wifi_api_action_deinit() ;
	} else {
		wifi_on_check = 1;
	}
	return status;
}

wmg_status_t wifi_off(void)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	if(wifi_on_check == 0)
		return OS_NET_STATUS_OK;

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_OFF, 0, 1, &status);

	wifi_api_action_deinit();

	wifi_on_check = 0;

	return status;
}

wmg_status_t wifi_sta_connect(wifi_sta_cn_para_t *cn)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_STA_CONNECT, 1, 1, cn, &status);

	return status;
}

wmg_status_t wifi_sta_disconnect(void)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_STA_DISCONNECT, 0, 1, &status);

	return status;
}

wmg_status_t wifi_sta_auto_reconnect(wmg_bool_t enable)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_AUTO_RECONNECT, 1, 1, enable, &status);

	return status;
}

wmg_status_t wifi_sta_get_info(wifi_sta_info_t *sta_info)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_STA_GET_INFO, 1, 1, sta_info, &status);

	return status;
}

wmg_status_t wifi_sta_list_networks(wifi_sta_list_t *sta_list)
{
	WIFI_STATE_CHECK(status);
	WMG_DEBUG("list network not support\n");
	return WMG_STATUS_NOT_READY;
}

wmg_status_t wifi_sta_remove_networks(char *ssid)
{
	WIFI_STATE_CHECK(status);
	WMG_DEBUG("remove network not support\n");
	return WMG_STATUS_NOT_READY;
}

wmg_status_t wifi_ap_enable(wifi_ap_config_t *ap_config)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_AP_ENABLE, 1, 1, ap_config, &status);

	return status;
}

wmg_status_t wifi_ap_disable(void)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

	act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_AP_DISABLE, 0, 1, &status);

	return status;
}

wmg_status_t wifi_ap_get_config(wifi_ap_config_t *ap_config)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_AP_GET_CONFIG, 1, 1, ap_config, &status);

	return status;
}

wmg_status_t wifi_monitor_enable(uint8_t channel)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_MON_ENABLE, 1, 1, channel, &status);

	return status;
}

wmg_status_t wifi_monitor_set_channel(uint8_t channel)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_MON_SET_CH, 1, 1, channel, &status);

	return status;
}

wmg_status_t wifi_monitor_disable(void)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_MON_DISABLE, 0, 1, &status);

	return status;
}

wmg_status_t wifi_set_scan_param(wifi_scan_param_t *scan_param)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_SET_SCAN_PARAM, 1, 1, scan_param, &status);

	return status;
}

wmg_status_t wifi_get_scan_results(wifi_scan_result_t *result, uint32_t *bss_num,
		uint32_t arr_size)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_GET_SCAN_RESULTS, 3, 1, result,
			bss_num, arr_size, &status);

	return status;
}

wmg_status_t wifi_set_mac(const char *ifname, uint8_t *mac_addr)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_SET_MAC, 2, 1,
			ifname, mac_addr,  &status);

	return status;
}

wmg_status_t wifi_get_mac(const char *ifname, uint8_t *mac_addr)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_GET_MAC, 2, 1,
			ifname, mac_addr, &status);

	return status;
}

wmg_status_t wifi_send_80211_rframe(uint8_t *data, uint32_t len, void *priv)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_SEND_80211_FRAME, 3, 1, data,
			len, priv, &status);

	return status;
}

wmg_status_t wifi_set_country_code(const char *country_code)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_SET_COUNTRY_CODE, 1, 1, country_code, &status);

	return status;
}

wmg_status_t wifi_get_country_code(char *country_code)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_GET_COUNTRY_CODE, 1, 1, country_code, &status);

	return status;
}

wmg_status_t wifi_set_ps_mode(wmg_bool_t enable)
{
	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_SET_PS_MODE, 1, 1, enable, &status);

	return status;
}

wmg_status_t wifi_vendor_send_data(uint8_t *data, uint32_t len)
{

	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_VENDOR_SEND_DATA,
					2, 1, data, len, &status);

	return status;
}

wmg_status_t wifi_vendor_register_rx_cb(wifi_vend_cb_t cb)
{

	wmg_status_t status = WMG_STATUS_FAIL;

	WIFI_STATE_CHECK(status);

    act_transfer(&wmg_act_handle, ACT_ID_WMG, ACWIFI_VENDOR_REG_CB,
					1, 1, cb, &status);

	return status;
}
