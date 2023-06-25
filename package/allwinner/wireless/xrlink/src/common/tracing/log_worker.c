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
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "log_worker.h"
#include "log_queue_block.h"

struct worker_work {
    void *(*function)(void *);
    void *arg;
    void **ret;
};

static log_queue_t worker_queue;
static pthread_t worker_handle;

int log_worker_init()
{
    if (log_queue_init(&worker_queue) != 0) {
        return -1;
    }
    if (pthread_create(&worker_handle, NULL, log_worker_loop, NULL)) {
        return -1;
    }
    return 0;
}

int log_worker_deinit()
{
    log_queue_destroy(&worker_queue);
    pthread_cancel(worker_handle);
    return 0;
}

void *log_worker_loop(void *arg)
{
    struct worker_work work;
    struct worker_work *work_ptr = &work;

    // if (!&worker_queue)
    //     return;

    while (1) {
        log_queue_pop(&worker_queue, (void *)&work_ptr);
        if (work.function) {
            *work.ret = work.function(work.arg);
        }
    }
    log_queue_destroy(&worker_queue);
    return NULL;
}

int log_worker_schedule(void *(*function)(void *), void *arg, void **ret)
{
    struct worker_work work;

    if (!function) {
        return ENOEXEC;
    }

    work.function = function;
    work.arg = arg;
    work.ret = ret;
    if (log_queue_push(&worker_queue, &work) != 0) {
        return ENOMEM;
    }

    return 0;
}
