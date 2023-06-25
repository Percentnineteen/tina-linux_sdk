#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/times.h>
#include "log_core.h"
#include "os_net_utils.h"
#include "wifimg.h"
#include "wifi_daemon.h"

#define PROC_NAME "wifi_deamon"
#define PID_FILE_PATH "/tmp/var/run/"

#define UNIX_DOMAIN "/tmp/UNIX_WIFI.domain"
#define READ_FROM_CLIENT 0X01
#define WRITE_TO_CLIENT  0x02
#define DATA_BUF_SIZE    256
#define RES_LEN          30
#define LIST_NETWORK_DEFAUT_NUM   5

#define MONITOR_DEFAULT_CHANNEL   6

#define BAND_NOME     0
#define BAND_2_4G     1
#define BAND_5G       2

static deamon_object_t deamon_object;

static int cmd_handle_o(char *p)
{
	int ret = -1;
	char *ssid_p = NULL;
	char *psk_p = NULL;
	char *delim = " ";

	char ssid_buf[SSID_MAX_LEN] = "allwinner-ap";
	char psk_buf[PSK_MAX_LEN] = "Aa123456";
	wifi_ap_config_t ap_config;
	ap_config.ssid = ssid_buf;
	ap_config.psk = psk_buf;
	ap_config.sec = WIFI_SEC_WPA2_PSK;
	ap_config.channel = 6;

	WFD_DEBUG("cmd handle o: %s\n",p);
	if(!strncmp(p, "sta", 3)) {
		WFD_INFO("open sta mode\n");
		ret = wifi_on(WIFI_STATION);
		if (ret == 0) {
			deamon_object.deamon_current_mode = WIFI_STATION;
			WFD_INFO("wifi on sta mode success\n");
			return 0;
		} else {
			WFD_ERROR("wifi on sta mode failed\n");
			return -1;
		}
	} else if(!strncmp(p, "ap", 2)) {
		if(strlen(p) > 2) {
			memset(ssid_buf, '\0', sizeof(SSID_MAX_LEN));
			memset(psk_buf, '\0', sizeof(PSK_MAX_LEN));
			p = p + 2;
			ssid_p = strtok(p, delim);
			psk_p = strtok(NULL, delim);
			if(psk_p == NULL) {
				strcpy(ssid_buf, ssid_p);
				strcpy(psk_buf, "NULL");
				WFD_INFO("open ap(%s) mode without psk\n",ssid_p);
				ap_config.sec = WIFI_SEC_NONE;
			} else {
				strcpy(ssid_buf, ssid_p);
				strcpy(psk_buf, psk_p);
				WFD_INFO("open ap(%s) mode with psk(%s)\n",ssid_p, psk_p);
				if(strlen(psk_p) < 8){
					WFD_ERROR("psk is too short, needs to be greater than 8 characters\n");
					return -1;
				}
			}
		} else {
			WFD_INFO("open default ap mode\n");
		}
		ret = wifi_on(WIFI_AP);
		if (ret == 0) {
			deamon_object.deamon_current_mode = WIFI_AP;
			WFD_INFO("wifi on ap mode success\n");
		} else {
			WFD_ERROR("wifi on ap mode failed\n");
			return -1;
		}
		ret = wifi_ap_enable(&ap_config);
		if (ret == 0) {
			WFD_INFO("ap enable success, ssid=%s, psk=%s, sec=%d, channel=%d\n", ap_config.ssid, ap_config.psk, ap_config.sec, ap_config.channel);
		} else {
			WFD_ERROR("ap enable failed\n");
			return -1;
		}
		return 0;
	} else if(!strncmp(p, "monitor", 7)) {
		ret = wifi_on(WIFI_MONITOR);
		if (ret == 0) {
			deamon_object.deamon_current_mode = WIFI_MONITOR;
			WFD_INFO("wifi on monitor mode success\n");
		} else {
			WFD_ERROR("wifi on monitor mode failed\n");
			return -1;
		}
		ret = wifi_monitor_enable(MONITOR_DEFAULT_CHANNEL);
		if (ret == 0) {
			WFD_INFO("monitor enable success, channel=%d\n", MONITOR_DEFAULT_CHANNEL);
		} else {
			WFD_ERROR("ap enable failed\n");
			return -1;
		}
		return 0;
	} else {
		WFD_ERROR("Unsupport wifi mode %s\n", p);
		return -1;
	}
}

static int cmd_handle_f(char *p)
{
	WFD_DEBUG("cmd handle f: %s\n",p);
	WFD_INFO("wifimanger off now\n");
	wifi_off();
	return 0;
}

static uint32_t freq_to_channel(uint32_t freq)
{
	int band;
	uint32_t channel = 0;
	if((freq <= 4980) && (freq >= 4910)){
		band = BAND_5G;
	} else if((freq >= 2407) && (freq <= 2484)){
		band = BAND_2_4G;
	} else {
		band = BAND_NOME;
	}
	switch (band) {
	case BAND_2_4G:
		if(freq == 2484) {
			channel = 14;
		} else if(freq == 2407) {
			channel = 0;
		} else if((freq <= 2472) && (freq > 2407)){
			if(((freq - 2407) % 5) == 0) {
				channel = ((freq - 2407) / 5);
			} else {
				channel = 1000;
			}
		} else {
			channel = 1000;
		}
		break;
	case BAND_5G:
		if(((freq - 4000) % 5) == 0) {
			channel = ((freq - 4000) / 5);
		} else {
			channel = 1000;
		}
		break;
	case BAND_NOME:
	default:
		channel = 1000;
		break;
	}
	return channel;
}

static char * key_mgmt_to_char(char *key_mgmt_buf, wifi_secure_t key_mgmt)
{
	switch (key_mgmt) {
		case WIFI_SEC_WEP:
			strncpy(key_mgmt_buf, "WEP", 3);
			break;
		case WIFI_SEC_WPA_PSK:
			strncpy(key_mgmt_buf, "WPA_PSK", 7);
			break;
		case WIFI_SEC_WPA2_PSK:
			strncpy(key_mgmt_buf, "WPA2_PSK", 8);
			break;
		case WIFI_SEC_NONE:
		default:
			strncpy(key_mgmt_buf, "NONE", 4);
			break;
	}
	return key_mgmt_buf;
}

static int cmd_handle_s(char *p)
{
	WFD_DEBUG("cmd handle s: %s\n",p);

	int sc_clk_tck = sysconf(_SC_CLK_TCK);
	struct tms begin_tms, end_tms;
	clock_t begin, end;
	char scan_res_char[12] = {0};

	int i, bss_num = 0, ret = 0;
	wifi_scan_result_t scan_res[RES_LEN] = {0};
	begin = times(&begin_tms);
	ret = wifi_get_scan_results(scan_res, &bss_num, RES_LEN);
	end = times(&end_tms);
	if (ret == 0) {
		for (i = 0; i < bss_num; i++) {
			memset(scan_res_char, 0, 12);
			WFD_INFO("bss[%02d]: bssid=%s  ssid=%s  channel=%d(freq=%d)  rssi=%d  sec=%s\n",
				i, scan_res[i].bssid, scan_res[i].ssid, freq_to_channel(scan_res[i].freq),
				scan_res[i].freq, scan_res[i].rssi, key_mgmt_to_char(scan_res_char,
					scan_res[i].key_mgmt));
		}
		WFD_INFO("===Wi-Fi scan successful, total %d ap time %4f ms===\n",
				bss_num, (((end - begin) / (double)sc_clk_tck) * 1000));
		return 0;
	} else {
		WFD_WARNG("===Wi-Fi scan failed, time %4f ms===\n", (((end - begin) / (double)sc_clk_tck)) * 1000);
	}
	return -1;
}

static int cmd_handle_c(char *p)
{
	WFD_DEBUG("cmd handle c: %s\n",p);

	int sc_clk_tck = sysconf(_SC_CLK_TCK);
	struct tms begin_tms, end_tms;
	clock_t begin, end;

	char *ssid_p = NULL;
	char *psk_p = NULL;
	char *delim = " ";
	char ssid_buf[SSID_MAX_LEN] = {0};
	char psk_buf[PSK_MAX_LEN] = {0};
	int ret = -1;

	wifi_sta_cn_para_t cn_para;
	ssid_p = strtok(p, delim);
	psk_p = strtok(NULL, delim);
	if(psk_p == NULL) {
		strcpy(ssid_buf, ssid_p);
		cn_para.sec = WIFI_SEC_NONE;
		WFD_INFO("connect to sta(%s) without pask\n",ssid_p);
	} else {
		strcpy(ssid_buf, ssid_p);
		strcpy(psk_buf, psk_p);
		cn_para.sec = WIFI_SEC_WPA2_PSK;
		WFD_INFO("connect to sta(%s) with pask(%s)\n",ssid_p ,psk_p);
	}
	cn_para.ssid = ssid_buf;
	cn_para.password = psk_buf;
	cn_para.fast_connect = 0;
	begin = times(&begin_tms);
	ret = wifi_sta_connect(&cn_para);
	end = times(&end_tms);
	if (ret == 0) {
		WFD_INFO("===Wi-Fi connect successful,time %4f ms===\n", (((end - begin) / (double)sc_clk_tck)) * 1000);
		return 0;
	} else {
		WFD_ERROR("===Wi-Fi connect failed,time %4f ms===\n", (((end - begin) / (double)sc_clk_tck)) * 1000);
	}
	return -1;
}

static int cmd_handle_d(char *p)
{
	int ret = -1;
	WFD_DEBUG("cmd handle d: %s\n",p);
	ret = wifi_sta_disconnect();
	if (ret != WMG_STATUS_SUCCESS) {
		WFD_ERROR("wifi sta mode disconnect failed, please check if an ap is connected\n");
	} else {
		WFD_INFO("wifi sta mode disconnect the ap success!\n");
	}
	return ret;
}

static int cmd_handle_a(char *p)
{
	int ret = -1;
	WFD_DEBUG("cmd handle a: %s\n",p);

	if(!strncmp(p, "enable", 6)) {
		WFD_INFO("sta auto reconnect set enable\n");
		ret = wifi_sta_auto_reconnect(true);
		if (ret == 0) {
			WFD_INFO("wifi set auto reconnect enable success\n");
		} else {
			WFD_WARNG("wifi set auto reconnect enable failed\n");
		}
	} else if(!strncmp(p, "disable", 7)) {
		WFD_INFO("sta auto reconnect set disable\n");
		ret = wifi_sta_auto_reconnect(false);
		if (ret == 0) {
			WFD_INFO("wifi set auto reconnect disable success\n");
		} else {
			WFD_WARNG("wifi set auto reconnect disable failed\n");
		}
	} else {
		WFD_ERROR("sta auto reconnect don't support cmd\n");
	}
	return ret;
}

static int cmd_handle_l(char *p)
{
	wmg_status_t ret;
	wifi_sta_info_t wifi_sta_info;

	wifi_sta_list_t sta_list_networks;
	wifi_sta_list_nod_t sta_list_nod[LIST_NETWORK_DEFAUT_NUM] = {0};
	int entry = 0, i = 0;

	wifi_ap_config_t ap_config;
	char ssid_buf[SSID_MAX_LEN] = {0};
	char psk_buf[PSK_MAX_LEN] = {0};
	char device_list[STA_MAX_NUM][BSSID_MAX_LEN] = {""};

	WFD_DEBUG("cmd handle l: %s\n",p);
	if(!strncmp(p, "all", 3)) {
		WFD_INFO("list all saved ap config info\n");
		sta_list_networks.list_nod = sta_list_nod;
		sta_list_networks.list_num = LIST_NETWORK_DEFAUT_NUM;
		ret = wifi_sta_list_networks(&sta_list_networks);
		if(ret == WMG_STATUS_SUCCESS) {
			entry = sta_list_networks.list_num >= sta_list_networks.sys_list_num ? sta_list_networks.sys_list_num : sta_list_networks.list_num;
			if(entry != 0){
				WFD_INFO("network id / ssid / bssid / flags\n");
				for(i = 0; entry > 0; entry--){
					WFD_INFO("%d\t%s\t%s\t%s\n",sta_list_networks.list_nod[i].id,
												sta_list_networks.list_nod[i].ssid,
												sta_list_networks.list_nod[i].bssid,
												sta_list_networks.list_nod[i].flags);
					i++;
				}
			} else {
				WFD_INFO("System has no entry save\n");
			}
			return 0;
		} else {
			WFD_ERROR("get list info faile\n");
		}
	} else {
		if(deamon_object.deamon_current_mode == WIFI_STATION) {
			WFD_INFO("list connected ap\n");
			ret = wifi_sta_get_info(&wifi_sta_info);
			if(ret == WMG_STATUS_SUCCESS) {
				WFD_INFO("sta id: %d\n",wifi_sta_info.id);
				WFD_INFO("sta freq: %d\n",wifi_sta_info.freq);
				WFD_INFO("sta bssid: %x%x:%x%x:%x%x:%x%x:%x%x:%x%x\n",(wifi_sta_info.bssid[0] >> 4),
																	(wifi_sta_info.bssid[0] & 0xf),
																	(wifi_sta_info.bssid[1] >> 4),
																	(wifi_sta_info.bssid[1] & 0xf),
																	(wifi_sta_info.bssid[2] >> 4),
																	(wifi_sta_info.bssid[2] & 0xf),
																	(wifi_sta_info.bssid[3] >> 4),
																	(wifi_sta_info.bssid[3] & 0xf),
																	(wifi_sta_info.bssid[4] >> 4),
																	(wifi_sta_info.bssid[4] & 0xf),
																	(wifi_sta_info.bssid[5] >> 4),
																	(wifi_sta_info.bssid[5] & 0xf));
				WFD_INFO("sta ssid: %s\n",wifi_sta_info.ssid);
				WFD_INFO("sta mac_addr: %x%x:%x%x:%x%x:%x%x:%x%x:%x%x\n",(wifi_sta_info.mac_addr[0] >> 4),
																	(wifi_sta_info.mac_addr[0] & 0xf),
																	(wifi_sta_info.mac_addr[1] >> 4),
																	(wifi_sta_info.mac_addr[1] & 0xf),
																	(wifi_sta_info.mac_addr[2] >> 4),
																	(wifi_sta_info.mac_addr[2] & 0xf),
																	(wifi_sta_info.mac_addr[3] >> 4),
																	(wifi_sta_info.mac_addr[3] & 0xf),
																	(wifi_sta_info.mac_addr[4] >> 4),
																	(wifi_sta_info.mac_addr[4] & 0xf),
																	(wifi_sta_info.mac_addr[5] >> 4),
																	(wifi_sta_info.mac_addr[5] & 0xf));
				WFD_INFO("sta ip_addr: %d.%d.%d.%d\n",wifi_sta_info.ip_addr[0],
													wifi_sta_info.ip_addr[1],
													wifi_sta_info.ip_addr[2],
													wifi_sta_info.ip_addr[3]);
				if(wifi_sta_info.sec == WIFI_SEC_NONE){
					WFD_INFO("sta sec: WIFI_SEC_NONE\n");
				} else if(wifi_sta_info.sec == WIFI_SEC_WPA_PSK) {
					WFD_INFO("sta sec: WIFI_SEC_WPA_PSK\n");
				} else if(wifi_sta_info.sec == WIFI_SEC_WPA2_PSK) {
					WFD_INFO("sta sec: WIFI_SEC_WPA2_PSK\n");
				}
				return 0;
			}
		} else if(deamon_object.deamon_current_mode == WIFI_AP) {
			ap_config.ssid = ssid_buf;
			ap_config.psk = psk_buf;
			for (i = 0; i < STA_MAX_NUM; i++) {
				ap_config.dev_list[i] = device_list[i];
			};
			ret = wifi_ap_get_config(&ap_config);
			if (ret == 0) {
				WFD_INFO("get ap config success:\n");
				WFD_INFO("ssid=%s\n", ap_config.ssid);
				WFD_INFO("psk=%s\n", ap_config.psk);
				WFD_INFO("sec=%d\n", ap_config.sec);
				WFD_INFO("channel=%d\n", ap_config.channel);
				WFD_INFO("sta_num=%d\n", ap_config.sta_num);
				for (i = 0; i < ap_config.sta_num; i++) {
					WFD_INFO("device%d: %s\n", i + 1, ap_config.dev_list[i]);
				}
				return 0;
			} else {
				WFD_ERROR("Error: get ap config failed\n");
			}
		} else {
			WFD_ERROR("current mode is unknow, please wifi on first\n");
		}
	}
	return -1;
}

static int cmd_handle_r(char *p)
{
	wmg_status_t ret = WMG_STATUS_FAIL;
	WFD_DEBUG("cmd handle r: %s\n",p);
	if(!strncmp(p, "all", 3)) {
		WFD_INFO("remove all saved networks\n");
		ret = wifi_sta_remove_networks(NULL);
		if (ret == WMG_STATUS_SUCCESS) {
			WFD_INFO("remove all networks success!\n");
		} else {
			WFD_ERROR("remove all networks faile\n");
		}
	} else {
		WFD_INFO("remove network(%s)\n", p);
		if(strlen(p) > SSID_MAX_LEN){
			WFD_ERROR("remove network(%s) longer than %d\n", p, SSID_MAX_LEN);
		} else {
			ret = wifi_sta_remove_networks(p);
			if (ret == WMG_STATUS_SUCCESS) {
				WFD_INFO("remove network(%s) success\n", p);
			} else {
				WFD_ERROR("remove network(%s) faile\n", p);
			}
		}
	}
	return ret;
}

static void uint8tochar(char *mac_addr_char, uint8_t *mac_addr_uint8)
{
	sprintf(mac_addr_char,"%02x:%02x:%02x:%02x:%02x:%02x",
						(unsigned char)mac_addr_uint8[0],
						(unsigned char)mac_addr_uint8[1],
						(unsigned char)mac_addr_uint8[2],
						(unsigned char)mac_addr_uint8[3],
						(unsigned char)mac_addr_uint8[4],
						(unsigned char)mac_addr_uint8[5]);
	mac_addr_char[17] = '\0';
}

static int cmd_handle_g(char *p)
{
	wmg_status_t ret = WMG_STATUS_FAIL;
	WFD_DEBUG("cmd handle g: %s\n",p);
	uint8_t mac_addr[6];
	char mac_addr_char[24] = {0};
	ret = wifi_get_mac(NULL, mac_addr);
	if (ret == WMG_STATUS_SUCCESS) {
		uint8tochar(mac_addr_char, mac_addr);
		WFD_INFO("get mac_addr: %s\n", mac_addr_char);
	}
	return ret;
}

static uint8_t char2uint8(char* trs)
{
	uint8_t ret = 0;
	uint8_t tmp_ret[2] = {0};
	int i = 0;
	for(; i < 2; i++) {
		switch (*(trs + i)) {
			case '0' :
				tmp_ret[i] = 0x0;
				break;
			case '1' :
				tmp_ret[i] = 0x1;
				break;
			case '2' :
				tmp_ret[i] = 0x2;
				break;
			case '3' :
				tmp_ret[i] = 0x3;
				break;
			case '4' :
				tmp_ret[i] = 0x4;
				break;
			case '5' :
				tmp_ret[i] = 0x5;
				break;
			case '6' :
				tmp_ret[i] = 0x6;
				break;
			case '7' :
				tmp_ret[i] = 0x7;
				break;
			case '8' :
				tmp_ret[i] = 0x8;
				break;
			case '9' :
				tmp_ret[i] = 0x9;
				break;
			case 'a' :
				tmp_ret[i] = 0xa;
				break;
			case 'b' :
				tmp_ret[i] = 0xb;
				break;
			case 'c' :
				tmp_ret[i] = 0xc;
				break;
			case 'd' :
				tmp_ret[i] = 0xd;
				break;
			case 'e' :
				tmp_ret[i] = 0xe;
				break;
			case 'f' :
				tmp_ret[i] = 0xf;
		break;
	}
	WFD_DEBUG("change num[%d]: %d\n", i, tmp_ret[i]);
	}
	ret = ((tmp_ret[0] << 4) | tmp_ret[1]);
	return ret;
}


static int cmd_handle_m(char *p)
{
	wmg_status_t ret = WMG_STATUS_FAIL;
	WFD_DEBUG("cmd handle m: %s\n",p);
	uint8_t mac_addr[6] = {0};
	char mac_addr_char[24] = {0};
	char *pch;
	int i;

	pch = strtok(p, ":");
	for(i = 0;(pch != NULL) && (i < 6); i++){
		mac_addr[i] = char2uint8(pch);
		pch = strtok(NULL, ":");
	}

	if(i != 6) {
		WFD_DEBUG("%s: mac address format is incorrect\n",p);
		return -1;
	}

	ret = wifi_set_mac(NULL, mac_addr);
	if (ret == WMG_STATUS_SUCCESS) {
		uint8tochar(mac_addr_char, mac_addr);
		WFD_INFO("set mac_addr: %s sucess\n", mac_addr_char);
	} else {
		uint8tochar(mac_addr_char, mac_addr);
		WFD_INFO("set mac_addr: %s faile\n", mac_addr_char);
	}
	return ret;
}

static int cmd_handle_p(char *p)
{
	WFD_INFO("Not support\n");
	return 0;
}

static int cmd_handle_D(char *p)
{
	int ret = -1;
	int level = -1;

	WFD_DEBUG("cmd handle D: %s\n",p);
	if(!strncmp(p, "error", 5)) {
		WFD_INFO("set debug level: error\n");
		level = 1;
	} else if(!strncmp(p, "warn", 4)) {
		WFD_INFO("set debug level: warn\n");
		level = 2;
	} else if(!strncmp(p, "info", 4)) {
		WFD_INFO("set debug level: info\n");
		level = 3;
	} else if(!strncmp(p, "debug", 5)) {
		WFD_INFO("set debug level: debug\n");
		level = 4;
	} else if(!strncmp(p, "dump", 4)) {
		WFD_INFO("set debug level: dump\n");
		level = 5;
	} else if(!strncmp(p, "exce", 4)) {
		WFD_INFO("set debug level: exce\n");
		level = 6;
	} else {
		WFD_ERROR("Don't support debug level:%s\n", p);
		goto set_erro;
	}

	ret = log_set_debug_level(level);
	if(ret) {
		printf("set debug level faile\n");
	}

set_erro:
	return ret;
}

const char XR_RAW_TEST_HANDLE[] = "raw_data : ";

void cmd_v_echo_printf(uint8_t *data, uint32_t len)
{
    if (strncmp(data,XR_RAW_TEST_HANDLE,strlen(XR_RAW_TEST_HANDLE)) == 0)
    {
        WFD_INFO("recv: %s\n",data);
    }
}

static int cmd_handle_v(char *p)
{
    static char xr_cb_flag = 0;

    if(xr_cb_flag == 0) {
        wifi_vendor_register_rx_cb(cmd_v_echo_printf);
        xr_cb_flag = 1;
    }

    char *raw_test = malloc(strlen(XR_RAW_TEST_HANDLE) +
                            strlen(p) + 1);
    strncpy(raw_test, XR_RAW_TEST_HANDLE, (strlen(XR_RAW_TEST_HANDLE) + 1));
    strcat(raw_test,p);
    WFD_INFO("send %s\n",raw_test);

    wifi_vendor_send_data(raw_test,(strlen(raw_test) + 1));
    free(raw_test);
    return 0;
}

void *cmd_handle_thread(void *arg)
{
	char *cmd_data = (char *)arg;
	WFD_DEBUG("get cmd: %s\n", cmd_data);
	pthread_detach(pthread_self());
	char *p = cmd_data;
	char ch = *p;
	p = p+2;
	switch (ch) {
		case 'o':
			cmd_handle_o(p);
		break;
		case 'f':
			cmd_handle_f(p);
		break;
		case 's':
			cmd_handle_s(p);
		break;
		case 'c':
			cmd_handle_c(p);
		break;
		case 'd':
			cmd_handle_d(p);
		break;
		case 'a':
			cmd_handle_a(p);
		break;
		case 'l':
			cmd_handle_l(p);
		break;
		case 'r':
			cmd_handle_r(p);
		break;
		case 'g':
			cmd_handle_g(p);
		break;
		case 'm':
			cmd_handle_m(p);
		break;
		case 'p':
			cmd_handle_p(p);
		break;
		case 'D':
			cmd_handle_D(p);
		break;
		case 'v':
			cmd_handle_v(p);
		break;
		default:
			WFD_ERROR("Un't support cmd: %c\n",ch);
		break;
	}
	free(arg);
	pthread_exit(0);
}

static int lockFile(int fd)
{
	struct flock fl;
	fl.l_type   = F_WRLCK;
	fl.l_start  = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len    = 0;
	fl.l_pid = -1;

	return(fcntl(fd, F_SETLK, &fl));
}

static int isRunning(const char *procname)
{
	char buf[16] = {0};
	char filename[128] = {0};
	sprintf(filename, "%s%s.pid",PID_FILE_PATH,procname);
	int fd = open(filename, O_CREAT|O_RDWR, 0600);
	if (fd < 0) {
		WFD_ERROR("open lockfile %s failed!\n", filename);
		return 1;
	}
	if (-1 == lockFile(fd))
	{
		WFD_WARNG("%s is already running\n", procname);
		close(fd);
		return 1;
	} else {
		ftruncate(fd, 0);
		sprintf(buf, "%ld", (long)getpid());
		write(fd, buf, strlen(buf) + 1);
		return 0;
	}
}
const char* wifi_event_to_str(wifi_sta_event_t event)
{
	switch(event) {
		case WIFI_DISCONNECTED:
			return "WIFI_DISCONNECTED";
		case WIFI_SCAN_STARTED:
			return "WIFI_SCAN_STARTED";
		case WIFI_SCAN_FAILED:
			return "WIFI_SCAN_FAILED";
		case WIFI_SCAN_RESULTS:
			return "WIFI_SCAN_RESULTS";
		case WIFI_NETWORK_NOT_FOUND:
			return "WIFI_NETWORK_NOT_FOUND";
		case WIFI_PASSWORD_INCORRECT:
			return "WIFI_PASSWORD_INCORRECT";
		case WIFI_AUTHENTIACATION:
			return "WIFI_AUTHENTIACATION";
		case WIFI_AUTH_REJECT:
			return "WIFI_AUTH_REJECT";
		case WIFI_ASSOCIATING:
			return "WIFI_ASSOCIATING";
		case WIFI_ASSOC_REJECT:
			return "WIFI_ASSOC_REJECT";
		case WIFI_ASSOCIATED:
			return "WIFI_ASSOCIATED";
		case WIFI_4WAY_HANDSHAKE:
			return "WIFI_4WAY_HANDSHAKE";
		case WIFI_GROUNP_HANDSHAKE:
			return "WIFI_GROUNP_HANDSHAKE";
		case WIFI_GROUNP_HANDSHAKE_DONE:
			return "WIFI_GROUNP_HANDSHAKE_DONE";
		case WIFI_CONNECTED:
			return "WIFI_CONNECTED";
		case WIFI_CONNECT_TIMEOUT:
			return "WIFI_CONNECT_TIMEOUT";
		case WIFI_DEAUTH:
			return "WIFI_DEAUTH";
		case WIFI_DHCP_START:
			return "WIFI_DHCP_START";
		case WIFI_DHCP_TIMEOUT:
			return "WIFI_DHCP_TIMEOUT";
		case WIFI_DHCP_SUCCESS:
			return "WIFI_DHCP_SUCCESS";
		case WIFI_TERMINATING:
			return "WIFI_TERMINATING";
		default:
			return "WIFI_EVENT_UNKNOWN";
	}
}

const char* wifi_state_to_str(wifi_sta_state_t state)
{
	switch(state) {
		case WIFI_STA_IDLE:
			return "WIFI_STA_IDLE";
		case WIFI_STA_CONNECTING:
			return "WIFI_STA_CONNECTING";
		case WIFI_STA_CONNECTED:
			return "WIFI_STA_CONNECTED";
		case WIFI_STA_OBTAINING_IP:
			return "WIFI_STA_OBTAINING_IP";
		case WIFI_STA_NET_CONNECTED:
			return "WIFI_STA_NET_CONNECTED";
		case WIFI_STA_DHCP_TIMEOUT:
			return "WIFI_STA_DHCP_TIMEOUT";
		case WIFI_STA_DISCONNECTING:
			return "WIFI_STA_DISCONNECTING";
		case WIFI_STA_DISCONNECTED:
			return "WIFI_STA_DISCONNECTED";
	}
}

void wifi_msg_cb(wifi_msg_data_t *msg)
{
	if(!msg)
		return;

	switch(msg->id) {
		case WIFI_MSG_ID_DEV_STATUS:
			 WFD_INFO("Wi-Fi dev status: %s\n",
					 msg->data.d_status == WLAN_STATUS_UP ? "UP" : "DOWN");
			 break;

		case WIFI_MSG_ID_STA_CN_EVENT:
			 WFD_INFO("Wi-Fi event:%s\n", wifi_event_to_str(msg->data.event));
			 break;

		case WIFI_MSG_ID_STA_STATE_CHANGE:
			 WFD_INFO("Wi-Fi state change --> %s\n", wifi_state_to_str(msg->data.state));
			 break;

		case WIFI_MSG_ID_AP_CN_EVENT:
		case WIFI_MSG_ID_MONITOR:
		default:
			 break;
	}
}

int main(void)
{
	pthread_t tid;
	pid_t pc;
	socklen_t clt_addr_len;
	int listen_fd;
	int com_fd;
	int ret;
	char data_buf[DATA_BUF_SIZE];
	char *pthread_arg = NULL;
	int len;
	struct sockaddr_un clt_addr;
	struct sockaddr_un srv_addr;
	int debug_level;

	pc = fork();
	if(pc<0)
	{
		printf("error fork\n");
		exit(-1);
	} else if(pc>0) {
		exit(0);
	}

	log_set_debug_level(LOG_LEVEL_INFO);
	WFD_INFO("set to debug level: %d\n", LOG_LEVEL_INFO);

	listen_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(listen_fd < 0) {
		perror("cannot create communication socket");
		return 1;
	}

	//set server addr_param
	srv_addr.sun_family = AF_UNIX;
	strcpy(srv_addr.sun_path,UNIX_DOMAIN);
	unlink(UNIX_DOMAIN);

	//bind sockfd & addr
	ret = bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
	if(ret == -1) {
		perror("cannot bind server socket");
		close(listen_fd);
		unlink(UNIX_DOMAIN);
		return 1;
	}

	//listen sockfd
	ret = listen(listen_fd,1);
	if(ret == -1) {
		perror("cannot listen the client connect request");
		close(listen_fd);
		unlink(UNIX_DOMAIN);
		return 1;
	}

	wifi_sta_auto_reconnect(true);

	wifi_register_msg_cb(wifi_msg_cb);

	while(1) {
		//have connect request use accept
		len = sizeof(clt_addr);
		com_fd = accept(listen_fd,(struct sockaddr*)&clt_addr,&len);
		if(com_fd < 0) {
			perror("cannot accept client connect request");
			close(listen_fd);
			unlink(UNIX_DOMAIN);
			return 1;
		}

		memset(data_buf, 0, DATA_BUF_SIZE);
		read(com_fd, data_buf, sizeof(data_buf));

		pthread_arg = (char *)malloc(DATA_BUF_SIZE);
		if(pthread_arg == NULL) {
			WFD_ERROR("malloc pthread_arg failed\n");
			return -1;
		}

		memset(pthread_arg, 0, DATA_BUF_SIZE);
		memcpy(pthread_arg, data_buf, DATA_BUF_SIZE);

		ret = pthread_create(&tid, NULL, cmd_handle_thread, (void *)pthread_arg);
		if (ret) {
			WFD_ERROR("failed to create handle thread: %s\n", strerror(ret));
			return -1;
		}

		close(com_fd);
	}

	close(listen_fd);
	unlink(UNIX_DOMAIN);
	return 0;
}

static deamon_object_t deamon_object = {
	.deamon_current_mode = WIFI_STATION,
};
