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


#ifndef __WLAN_CMD_H__
#define __WLAN_CMD_H__
#include "xr_proto.h"

os_net_status_t xr_wifi_init(void);

os_net_status_t xr_wifi_deinit(void);

os_net_status_t xr_wifi_on(xr_wifi_mode_t mode);

os_net_status_t xr_wifi_off(void);

os_net_status_t xr_wifi_sta_connect(xr_wifi_sta_cn_t *cn_para);

os_net_status_t xr_wifi_sta_disconnect(void);

os_net_status_t xr_wifi_sta_auto_reconnect(xr_bool_t en);

os_net_status_t xr_wifi_sta_get_info(xr_wifi_sta_info_t *sta_info);

os_net_status_t xr_wifi_ap_enable(xr_wifi_ap_config_t *ap_config);

os_net_status_t xr_wifi_ap_disable(void);

os_net_status_t xr_wifi_ap_get_config_info(xr_wifi_ap_info_t *info);

os_net_status_t xr_wifi_monitor_enable(xr_wifi_monitor_enable_t *m);

os_net_status_t xr_wifi_monitor_disable(void);

os_net_status_t xr_wifi_get_scan_results(void);

os_net_status_t xr_wifi_set_mac(xr_wifi_mac_info_t *mi);

os_net_status_t xr_wifi_get_mac(xr_wifi_mac_info_t *mi);

os_net_status_t xr_wifi_send_80211_frame(xr_wifi_send_raw_data_t *raw);

typedef void (*xr_wifi_msg_cb_t)(char *data, uint32_t len);

os_net_status_t xr_wifi_register_msg_cb(xr_wifi_msg_cb_t cb);

os_net_status_t xr_wifi_sta_list_networks(void);

os_net_status_t xr_wifi_sta_remove_networks(char *data);

os_net_status_t xr_wifi_set_country_code(xr_wifi_country_code_info_t *code);

os_net_status_t xr_wifi_get_country_code(void);

os_net_status_t xr_wifi_send_raw_data(xr_wifi_raw_data_t *data);

#define CMD_HEAD_LEN (sizeof(xr_cfg_payload_t))

#define CMD_LOG_TAG "XR_CMD"

#define CMD_INFO(fmt,arg...) \
	log_print(CMD_LOG_TAG,LOG_LEVEL_INFO,fmt,##arg)

#define CMD_DEBUG(fmt,arg...) \
	log_print(CMD_LOG_TAG,LOG_LEVEL_DEBUG,fmt,##arg)

#define CMD_WARNG(fmt,arg...) \
	log_print(CMD_LOG_TAG,LOG_LEVEL_WARNING,fmt,##arg)

#define CMD_ERROR(fmt,arg...) \
	log_print(CMD_LOG_TAG,LOG_LEVEL_ERROR,fmt,##arg)

#endif
