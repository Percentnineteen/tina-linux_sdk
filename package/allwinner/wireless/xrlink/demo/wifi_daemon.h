/*
 * Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 */

#ifndef __WIFI_DAEMON_H__
#define __WIFI_DAEMON_H__

#if __cplusplus
extern "C" {
#endif

typedef struct {
	wifi_mode_t deamon_current_mode;
} deamon_object_t;

#include "log_core.h"

#define CMD_LOG_TAG ""

#define WFD_INFO(fmt,arg...) \
	log_print(CMD_LOG_TAG,LOG_LEVEL_INFO,fmt,##arg)

#define WFD_DEBUG(fmt,arg...) \
	log_print(CMD_LOG_TAG,LOG_LEVEL_DEBUG,fmt,##arg)

#define WFD_WARNG(fmt,arg...) \
	log_print(CMD_LOG_TAG,LOG_LEVEL_WARNING,fmt,##arg)

#define WFD_ERROR(fmt,arg...) \
	log_print(CMD_LOG_TAG,LOG_LEVEL_ERROR,fmt,##arg)


#if __cplusplus
}
#endif

#endif /* __WIFI_DAEMON_H__ */
