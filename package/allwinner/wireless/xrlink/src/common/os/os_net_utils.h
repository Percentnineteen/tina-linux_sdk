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


#ifndef __OS_NET_UTILS_H__
#define __OS_NET_UTILS_H__
#include <stdio.h>
#include <stdlib.h>

#include "log_core.h"

#define OS_LOG_TAG "OS_NET_UTIS"

typedef enum {
    OS_NET_STATUS_OK = 0,
    OS_NET_STATUS_FAILED,
    OS_NET_STATUS_NOT_READY,
    OS_NET_STATUS_BUSY,
    OS_NET_STATUS_PARAM_INVALID,
    OS_NET_STATUS_NOMEM,
} os_net_status_t;

#define OS_NET_INFO(fmt, arg...) \
	log_print(OS_LOG_TAG,LOG_LEVEL_INFO,fmt,##arg)

#define OS_NET_DEBUG(fmt, arg...) \
	log_print(OS_LOG_TAG,LOG_LEVEL_DEBUG,fmt,##arg)

#define OS_NET_ERROR(fmt, arg...) \
	log_print(OS_LOG_TAG,LOG_LEVEL_ERROR,fmt,##arg)

#define OS_NET_WARN(fmt, arg...) \
	log_print(OS_LOG_TAG,LOG_LEVEL_WARNING,fmt,##arg)

#define OS_NET_DUMP(fmt, arg...) \
	log_print(OS_LOG_TAG,LOG_LEVEL_MSGDUMP,fmt,##arg)

#define OS_NET_EXCESSIVE(fmt, arg...) \
	log_print(OS_LOG_TAG,LOG_LEVEL_EXCESSIVE,fmt,##arg)

#define OS_NET_TASK_POST_BLOCKING (0xffffffffU)
#define OS_NET_TASK_POST_NO_BLOCKING (0)
#define OS_NET_WAIT_FOREVER (0xffffffffU)
#endif
