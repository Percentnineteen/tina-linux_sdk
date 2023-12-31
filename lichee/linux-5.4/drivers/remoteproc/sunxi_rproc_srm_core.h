 /*
* linux/drivers/remoteproc/sunxi_rproc_srm_core.h
*
* Copyright © 2020-2025, wujiayi
*              Author: wujiayi <wujiayi@allwinnertech.com>
*
* This file is provided under a dual BSD/GPL license.  When using or
* redistributing this file, you may do so under either license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#ifndef _RPROC_SRM_CORE_H_
#define _RPROC_SRM_CORE_H_

/**
 * Message type used in resource manager rpmsg:
 *  RPROC_SRM_MSG_GETCONFIG:    Request to get the configuration of a resource
 *  RPROC_SRM_MSG_SETCONFIG:    Request to set the configuration of a resource
 *  RPROC_SRM_MSG_INIT_TRIGGER: Trigger to init all peripheral
 *  RPROC_SRM_MSG_ERROR:        Error when processing a request
 */

#define RPROC_SRM_MSG_GETCONFIG		0x00
#define RPROC_SRM_MSG_SETCONFIG		0x01
#define RPROC_SRM_MSG_INIT_TRIGGER	0x02
#define RPROC_SRM_MSG_ERROR		0xFF

/**
 * Resource type used in resource manager rpmsg:
 *  RPROC_SRM_RSC_CLOCK:        clock resource
 *  RPROC_SRM_RSC_REGU:         regulator resource
 */
#define RPROC_SRM_RSC_CLOCK     0x00
#define RPROC_SRM_RSC_REGU      0x01

/**
 * struct clock_cfg - clock configuration used in resource manager rpmsg
 * @index:      clock index
 * @name:       clock name
 * @rate:       clock rate request (in SetConfig message) or current status (in
 *              GetConfig message)
 */
struct clock_cfg {
	u32 index;
	u8 name[16];
	u32 rate;
};

/**
 * struct regu_cfg - regu configuration used in resource manager rpmsg
 * @index:      regulator index
 * @name:       regulator name
 * @enable:     regulator enable/disable request (in SetConfig message) or
 *              current status (in GetConfig message)
 * @curr_voltage_mv: current regulator voltage in mV (meaningful in
 *                   SetConfig message)
 * @min_voltage_mv:  regulator min voltage request in mV (meaningful in
 *                   SetConfig message)
 * @max_voltage_mv:  regulator max voltage request in mV (meaningful in
 *                   SetConfig message)
 */
struct regu_cfg {
	u32 index;
	u8 name[16];
	u32 enable;
	u32 curr_voltage_mv;
	u32 min_voltage_mv;
	u32 max_voltage_mv;
};

/**
 * struct rpmsg_srm_msg - message structure used between processors to
 *                        dynamically update resources configuration
 * @message_type: type of the message: see RPROC_SRM_MSG*
 * @device_id:  an identifier specifying the device owning the resources.
 *              This is implementation dependent. As example it may be the
 *              device name or the device address.
 * @rsc_type:   the type of the resource for which the configuration applies:
 *              see RPROC_SRM_RSC*
 */
struct rpmsg_srm_msg {
	u32 message_type;
	u8 device_id[32];
	u32 rsc_type;
};

struct rpmsg_srm_msg_desc {
	struct rpmsg_endpoint *ept;
	struct rpmsg_srm_msg *msg;
};

struct rproc_srm_core;

int rproc_srm_core_register_notifier(struct rproc_srm_core *core,
				     struct notifier_block *nb);
int rproc_srm_core_unregister_notifier(struct rproc_srm_core *core,
				       struct notifier_block *nb);
int rpmsg_srm_send(struct rpmsg_endpoint *ept, struct rpmsg_srm_msg *msg);

#endif
