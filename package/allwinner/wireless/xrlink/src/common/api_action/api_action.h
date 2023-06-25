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


#ifndef __API_ACTION_H_
#define __API_ACTION_H_

#include <os_net_utils.h>
#include <os_net_queue.h>
#include <os_net_sem.h>
#include <os_net_mutex.h>
#include <os_net_thread.h>

typedef struct {
    int (*action)(void **call_argv, void **cb_argv);
    const char *name;
} act_func_t;

typedef uint32_t act_post_timeout_t;

typedef struct {
    os_net_queue_t queue;
    os_net_thread_t thread;
    os_net_thread_pid_t pid;
    os_net_sem_t sem;
    os_net_recursive_mutex_t mutex;
    bool enable;
} act_handle_t;

/*
 * 1.ACT_FUNC_NUM is currently set to 20
 * 2.bt   use 0 ~ 9 table number
 * 3.wifi use 10 ~ 19 table number
 * 4.If other modules want to use the api action, needs to define the table number
 */
#define ACT_FUNC_NUM 20

os_net_status_t act_register_handler(act_handle_t *handle, uint8_t id, act_func_t *action);

os_net_status_t act_init(act_handle_t *handle, const char *identifier);

os_net_status_t act_deinit(act_handle_t *handle);

os_net_status_t act_transfer(act_handle_t *handle, uint8_t id, uint16_t opcode, int call_arg_len,
                             int cb_arg_len, ...);

#define OS_NET_TASK_QUEUE_LEN (24)
#define OS_NET_TASK_STACK_SIZE (4096)
#define OS_NET_TASK_PRIO (0)

#endif /*__ACTION_H_*/
