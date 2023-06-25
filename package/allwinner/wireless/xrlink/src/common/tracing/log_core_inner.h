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


#ifndef __LOG_CORE_INNER_H__
#define __LOG_CORE_INNER_H__

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#if __cplusplus
extern "C" {
#endif

#define LOG_ERROR(fmt, arg...)                                                                     \
    log_print("LOG_CORE", LOG_LEVEL_ERROR, "[%s:%d]:  " fmt "\n", __func__, __LINE__, ##arg)
#define LOG_WARNING(fmt, arg...)                                                                   \
    log_print("LOG_CORE", LOG_LEVEL_WARNING, "[%s:%d]:  " fmt "\n", __func__, __LINE__, ##arg)
#define LOG_INFO(fmt, arg...)                                                                      \
    log_print("LOG_CORE", LOG_LEVEL_INFO, "[%s:%d]:  " fmt "\n", __func__, __LINE__, ##arg)
#define LOG_DEBUG(fmt, arg...)                                                                     \
    log_print("LOG_CORE", LOG_LEVEL_DEBUG, "[%s:%d]:  " fmt "\n", __func__, __LINE__, ##arg)

#if __cplusplus
}; // extern "C"
#endif

#endif // __LOG_CORE_INNER_H__
