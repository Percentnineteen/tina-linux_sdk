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
#include <linux/netlink.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "msg_netlink.h"

static int nlmsg_recv(nlmsg_priv_t *priv)
{
	struct nlmsghdr *nlh = NULL;
	struct sockaddr_nl src_addr;
	struct iovec iov;
	int len = 0;

	struct msghdr msg;

	nlh = (struct nlmsghdr*) priv->rx;

	iov.iov_base = (void*) nlh;
	iov.iov_len = NLMSG_SPACE(TX_RX_MAX_LEN);
	memset(&src_addr, 0, sizeof(struct sockaddr_nl));

	memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = (void *)&src_addr;
	msg.msg_namelen = sizeof(struct sockaddr_nl);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	len = recvmsg(priv->fd, &msg, 0);
	if(len < 0) {
		LINK_ERROR("recvmsg error!\n");
		return -1;
	}

	len = nlh->nlmsg_len - NLMSG_SPACE(0);
	if(len < 0) {
		LINK_ERROR("len < 0 !\n");
		return -1;
	}

	if(len > TX_RX_MAX_LEN) {
		LINK_ERROR("len overflow!\n");
	}

	memcpy(priv->rx, (char *)NLMSG_DATA(nlh), len);
	return len;
}

static void *nlmsg_thread_handle(void *arg)
{
	int len = 0;

	LINK_DEBUG("nlmsg thread start.\n");

	nlmsg_priv_t *priv = (nlmsg_priv_t *) arg;
	for(;;) {
		len = nlmsg_recv(priv);
		if(len > 0) {
			if(priv->recv_cb) {
				priv->recv_cb(priv->rx,len);
			}
		}
	}

	return NULL;
}

nlmsg_priv_t *nlmsg_init(recv_cb_t cb)
{
	nlmsg_priv_t *priv = NULL;
	struct sockaddr_nl addr;
	int fd = -1;
	os_net_thread_pid_t pid;
	int ret = -1;
	os_net_thread_get_pid(&pid);

	priv = (nlmsg_priv_t *)malloc(sizeof(nlmsg_priv_t));
	if(!priv) {
		LINK_ERROR("No memory.\n");
		return NULL;
	}

	memset(priv, 0, sizeof(nlmsg_priv_t));

	priv->rx = (char *)malloc(TX_RX_MAX_LEN);
	if(!priv->rx) {
		free(priv);
		return NULL;
	}

	priv->fd = socket(AF_NETLINK, SOCK_RAW, NLMSG_PROTO);
	if(!priv->fd) {
		LINK_ERROR("NLMSG create failed.\n");
		free(priv->rx);
		free(priv);
		return NULL;
	}

	memset(&addr, 0, sizeof(struct sockaddr_nl));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = (uint32_t)pid;
	addr.nl_groups = 0;

	ret = bind(priv->fd, (struct sockaddr *)&addr,
			sizeof(struct sockaddr_nl));
	if(ret < 0) {
		close(priv->fd);
		LINK_ERROR("NLMSG bind failed.\n");
		goto error;
	}

	priv->recv_cb = cb;

	if(os_net_thread_create(&priv->thread, "nlmsg", nlmsg_thread_handle,priv,0,4096) != OS_NET_STATUS_OK) {
		goto error;
	}
	return priv;
error:
	close(priv->fd);
	free(priv->rx);
	free(priv);
	return NULL;
}

os_net_status_t nlmsg_deinit(nlmsg_priv_t *priv)
{

	LINK_DEBUG("nlmsg_deinit\r\n");

	os_net_thread_delete(&priv->thread);

	if(priv->fd > 0) {
		close(priv->fd);
	}

	if(priv && priv->rx) {
		free(priv->rx);
	}

	if(priv) {
		free(priv);
	}

	return OS_NET_STATUS_OK;
}

os_net_status_t nlmsg_send(nlmsg_priv_t *priv, char *data, uint32_t len)
{
	struct nlmsghdr *nlh = NULL;
	int nlmsg_len = 0;
	struct msghdr msg;
	struct iovec iov;
	struct sockaddr_nl dest_addr;
	os_net_thread_pid_t pid;

	os_net_thread_get_pid(&pid);


	if(!priv || priv->fd < 0) {
		LINK_ERROR("handle is invaild, priv->fd=%d\n", priv->fd);
		return OS_NET_STATUS_PARAM_INVALID;
	}

	/*dest address*/
	memset(&dest_addr, 0, sizeof(struct sockaddr_nl));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; //for linux kernel
	dest_addr.nl_groups = 0;

	/*fill the netlink message header*/
	nlmsg_len = NLMSG_SPACE(len);

	nlh = (struct nlmsghdr *)malloc(nlmsg_len);

	memset(nlh, 0, nlmsg_len);
	nlh->nlmsg_len = nlmsg_len;
	nlh->nlmsg_flags = 0;
	nlh->nlmsg_pid = (uint32_t)pid; //self pid

	/*fill in the netlink message payload*/
	memcpy(NLMSG_DATA(nlh), data, len);

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;

	memset(&msg, 0, sizeof(struct msghdr));

	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(struct sockaddr_nl);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;

	//log_hex_dump(__func__,20,data,len);

	if(sendmsg(priv->fd, &msg, 0) <0) {
		LINK_ERROR("Send error.\n");
		free(nlh);
		return OS_NET_STATUS_FAILED;
	}
	free(nlh);
	return OS_NET_STATUS_OK;
}

