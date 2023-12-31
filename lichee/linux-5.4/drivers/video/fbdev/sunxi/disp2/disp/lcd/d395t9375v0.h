/* drivers/video/sunxi/disp2/disp/lcd/ST7796S.h
 *
 * Copyright (c) 2017 Allwinnertech Co., Ltd.
 * Author: zhengxiaobin <zhengxiaobin@allwinnertech.com>
 *         zhangyuanjings <zhangyuanjings@allwinnertech.com>
 *
 * d395t9375v0 panel driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef _D395T9375V0_PANEL_H
#define _D395T9375V0_PANEL_H

#include "panels.h"

extern struct __lcd_panel d395t9375v0_panel;

extern s32 bsp_disp_get_panel_info(u32 screen_id, struct disp_panel_para *info);

#endif /*End of file*/
