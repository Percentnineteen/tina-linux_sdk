/*
 * drivers/video/fbdev/sunxi/disp2/disp/fb_g2d_rot/fb_g2d_rot.c
 *
 * Copyright (c) 2007-2019 Allwinnertech Co., Ltd.
 * Author: zhengxiaobin <zhengxiaobin@allwinnertech.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "fb_g2d_rot.h"

extern int g2d_blit_h(g2d_blt_h *para);
extern  int g2d_open(struct inode *inode, struct file *file);
extern  int g2d_release(struct inode *inode, struct file *file);
extern void g2d_ioctl_mutex_lock(void);
extern void g2d_ioctl_mutex_unlock(void);

int fb_g2d_rot_free(struct fb_g2d_rot_t *inst)
{
	struct file g2d_file;

	if (inst) {
		disp_free((void *__force)inst->dst_vir_addr,
		  (void *)inst->dst_phy_addr, inst->dst_mem_len);
		kfree(inst);
		g2d_release(0, &g2d_file);
	}
	return 0;
}

int fb_g2d_set_degree(struct fb_g2d_rot_t *inst,
		      struct disp_layer_config *config, unsigned int degree)
{
	int degree_g2d = 0;

	if (inst) {
		switch (degree) {
		case FB_ROTATION_HW_0:
			degree_g2d = G2D_ROT_0;
			break;
		case FB_ROTATION_HW_90:
			degree_g2d = G2D_ROT_90;
			break;
		case FB_ROTATION_HW_180:
			degree_g2d = G2D_ROT_180;
			break;
		case FB_ROTATION_HW_270:
			degree_g2d = G2D_ROT_270;
			break;
		default:
			degree_g2d = -1;
			break;
		}
		switch (degree_g2d) {
		case G2D_ROT_90:
		case G2D_ROT_270:
			inst->info.flag_h = degree_g2d;
			inst->fb->var.rotate = degree;
			inst->info.dst_image_h.width =
			    inst->fb->var.yres;
			inst->info.dst_image_h.height =
			    inst->fb->var.xres;
			inst->info.dst_image_h.clip_rect.w =
			    inst->info.dst_image_h.height;
			inst->info.dst_image_h.clip_rect.h =
			    inst->info.dst_image_h.width;
			config->info.fb.crop.width =
			    ((long long)inst->fb->var.yres << 32);
			config->info.fb.crop.height =
			    ((long long)inst->fb->var.xres << 32);
			config->info.fb.size[0].width =
			    inst->fb->var.yres;
			config->info.fb.size[0].height =
			    inst->fb->var.xres;
			config->info.fb.size[1].width =
			    inst->fb->var.yres;
			config->info.fb.size[1].height =
			    inst->fb->var.xres;
			config->info.fb.size[2].width =
			    inst->fb->var.yres;
			config->info.fb.size[2].height =
			    inst->fb->var.xres;
			break;
		case G2D_ROT_0:
		case G2D_ROT_180:
			inst->info.flag_h = degree_g2d;
			inst->fb->var.rotate = degree;
			inst->info.dst_image_h.width =
			    inst->fb->var.xres;
			inst->info.dst_image_h.height =
			    inst->fb->var.yres;
			inst->info.dst_image_h.clip_rect.w =
			    inst->info.dst_image_h.width;
			inst->info.dst_image_h.clip_rect.h =
			    inst->info.dst_image_h.height;
			config->info.fb.crop.width =
			    ((long long)inst->fb->var.xres << 32);
			config->info.fb.crop.height =
			    ((long long)inst->fb->var.yres << 32);
			config->info.fb.size[0].width =
			    inst->fb->var.xres;
			config->info.fb.size[0].height =
			    inst->fb->var.yres;
			config->info.fb.size[1].width =
			    inst->fb->var.xres;
			config->info.fb.size[1].height =
			    inst->fb->var.yres;
			config->info.fb.size[2].width =
			    inst->fb->var.xres;
			config->info.fb.size[2].height =
			    inst->fb->var.yres;
			break;
		default:
			pr_warn("Not support degreee:%d\n", degree);
			break;
		}
	}
	return 0;
}

int fb_g2d_rot_apply(struct fb_g2d_rot_t *inst, struct disp_layer_config *config)
{
	int ret = -1;

	if (!inst || !config || !inst->fb) {
		pr_warn("%s:Null pointer\n", __func__);
		return ret;
	}

	switch (inst->info.flag_h) {
	case G2D_ROT_90:
		config->info.fb.crop.width =
		    ((long long)inst->fb->var.yres << 32);
		config->info.fb.crop.height =
		    ((long long)inst->fb->var.xres << 32);
		break;
	case G2D_ROT_0:
	case G2D_ROT_180:
		break;
	case G2D_ROT_270:
		config->info.fb.crop.width =
		    ((long long)inst->fb->var.yres << 32);
		config->info.fb.crop.height =
		    ((long long)inst->fb->var.xres << 32);
		break;
	default:
		pr_warn("Not support degree:%d\n", inst->info.flag_h);
		break;
	}
	inst->info.src_image_h.laddr[0] =
	    inst->fb->fix.smem_start +
	    inst->fb->fix.line_length * inst->fb->var.yoffset;


	g2d_ioctl_mutex_lock();
	ret = g2d_blit_h(&inst->info);
	if (ret)
		pr_warn("g2d_blit_h fail!ret:%d\n", ret);
	g2d_ioctl_mutex_unlock();

	if (inst->switch_buffer_flag == 0) {
		inst->info.dst_image_h.laddr[0] = (u32)inst->dst_phy_addr + inst->dst_mem_len/2;
		config->info.fb.addr[0] =
			(unsigned long long)inst->dst_phy_addr;
		++inst->switch_buffer_flag;
	} else {
		inst->info.dst_image_h.laddr[0] = (u32)inst->dst_phy_addr;
		config->info.fb.addr[0] =
			(unsigned long long)inst->dst_phy_addr + inst->dst_mem_len/2;
		inst->switch_buffer_flag = 0;
	}


	config->info.fb.crop.y =
		((unsigned long long)(0)) << 32;
	config->info.fb.crop.x =
		((unsigned long long)(0)) << 32;


	return ret;
}

struct fb_g2d_rot_t *fb_g2d_rot_create(struct fb_info *p_info,
				       unsigned int fb_id,
				       struct disp_layer_config *config)
{
	int ret = -1;
	unsigned int value = 0;
	char sub_name[32] = {0};
	struct fb_g2d_rot_t *fb_rot = NULL;
	struct file g2d_file;

	if (!p_info || !config) {
		pr_warn("%s:Null pointer\n", __func__);
		return NULL;
	}

	ret = disp_sys_script_get_item("disp", "disp_rotation_used", &value, 1);
	if (ret != 1)
		value = 0;

	if (!value) {
		pr_warn("rotation hw function is configed as no used\n");
		return NULL;
	}

	sprintf(sub_name, "degree%d", fb_id);
	ret = disp_sys_script_get_item("disp", sub_name, &value, 1);
	if (value == FB_ROTATION_HW_0 || ret != 1) {
		pr_warn("rotation hw function is configed to zero degree\n");
		return NULL;
	}

	fb_rot = kmalloc(sizeof(struct fb_g2d_rot_t), GFP_KERNEL | __GFP_ZERO);
	if (!fb_rot) {
		pr_warn("kmalloc fail!!\n");
		return NULL;
	}
	ret = g2d_open(0, &g2d_file);
	if (ret)
		goto ERROR;

	fb_rot->info.src_image_h.width = p_info->var.xres;
	fb_rot->info.src_image_h.height = p_info->var.yres;

	switch (value) {
	case FB_ROTATION_HW_90:
		fb_rot->info.flag_h = G2D_ROT_90;
		p_info->var.rotate = FB_ROTATION_HW_90;
		fb_rot->info.dst_image_h.width = p_info->var.yres;
		fb_rot->info.dst_image_h.height = p_info->var.xres;
		fb_rot->info.dst_image_h.clip_rect.w = fb_rot->info.dst_image_h.height;
		fb_rot->info.dst_image_h.clip_rect.h = fb_rot->info.dst_image_h.width;
		config->info.fb.crop.width = ((long long)p_info->var.yres << 32);
		config->info.fb.crop.height = ((long long)p_info->var.xres << 32);
		config->info.fb.size[0].width = p_info->var.yres;
		config->info.fb.size[0].height = p_info->var.xres;
		config->info.fb.size[1].width = p_info->var.yres;
		config->info.fb.size[1].height = p_info->var.xres;
		config->info.fb.size[2].width = p_info->var.yres;
		config->info.fb.size[2].height = p_info->var.xres;
		break;
	case FB_ROTATION_HW_180:
		fb_rot->info.flag_h = G2D_ROT_180;
		p_info->var.rotate = FB_ROTATION_HW_180;
		fb_rot->info.dst_image_h.width = p_info->var.xres;
		fb_rot->info.dst_image_h.height = p_info->var.yres;
		fb_rot->info.dst_image_h.clip_rect.w = fb_rot->info.dst_image_h.width;
		fb_rot->info.dst_image_h.clip_rect.h = fb_rot->info.dst_image_h.height;
		break;
	case FB_ROTATION_HW_270:
		fb_rot->info.flag_h = G2D_ROT_270;
		p_info->var.rotate = FB_ROTATION_HW_270;
		fb_rot->info.dst_image_h.width = p_info->var.yres;
		fb_rot->info.dst_image_h.height = p_info->var.xres;
		fb_rot->info.dst_image_h.clip_rect.w = fb_rot->info.dst_image_h.height;
		fb_rot->info.dst_image_h.clip_rect.h = fb_rot->info.dst_image_h.width;
		config->info.fb.crop.width = ((long long)p_info->var.yres << 32);
		config->info.fb.crop.height = ((long long)p_info->var.xres << 32);
		config->info.fb.size[0].width = p_info->var.yres;
		config->info.fb.size[0].height = p_info->var.xres;
		config->info.fb.size[1].width = p_info->var.yres;
		config->info.fb.size[1].height = p_info->var.xres;
		config->info.fb.size[2].width = p_info->var.yres;
		config->info.fb.size[2].height = p_info->var.xres;
		break;
	default:
		p_info->var.rotate = 0;
		pr_warn("Not support degree:%d\n", value);
		goto G2D_RELEASE;
		break;
	}

	switch (config->info.fb.format) {
	case DISP_FORMAT_ARGB_8888:
		fb_rot->info.src_image_h.format = G2D_FORMAT_ARGB8888;
		fb_rot->info.dst_image_h.format = G2D_FORMAT_ARGB8888;
		break;
	case DISP_FORMAT_ABGR_8888:
		fb_rot->info.src_image_h.format = G2D_FORMAT_ABGR8888;
		fb_rot->info.dst_image_h.format = G2D_FORMAT_ABGR8888;
		break;
	case DISP_FORMAT_RGBA_8888:
		fb_rot->info.src_image_h.format = G2D_FORMAT_RGBA8888;
		fb_rot->info.dst_image_h.format = G2D_FORMAT_RGBA8888;
		break;
	case DISP_FORMAT_BGRA_8888:
		fb_rot->info.src_image_h.format = G2D_FORMAT_BGRA8888;
		fb_rot->info.dst_image_h.format = G2D_FORMAT_BGRA8888;
		break;
	case DISP_FORMAT_RGB_888:
		fb_rot->info.src_image_h.format = G2D_FORMAT_RGB888;
		fb_rot->info.dst_image_h.format = G2D_FORMAT_RGB888;
		break;
	case DISP_FORMAT_BGR_888:
		fb_rot->info.src_image_h.format = G2D_FORMAT_BGR888;
		fb_rot->info.dst_image_h.format = G2D_FORMAT_BGR888;
		break;
	case DISP_FORMAT_RGB_565:
		fb_rot->info.src_image_h.format = G2D_FORMAT_RGB565;
		fb_rot->info.dst_image_h.format = G2D_FORMAT_RGB565;
		break;
	case DISP_FORMAT_BGR_565:
		fb_rot->info.src_image_h.format = G2D_FORMAT_BGR565;
		fb_rot->info.dst_image_h.format = G2D_FORMAT_BGR565;
		break;
	default:
		pr_warn("Not support pixel format %d\n", config->info.fb.format);
		goto ERROR;
		break;
	}

	fb_rot->info.src_image_h.laddr[0] = p_info->fix.smem_start;
	fb_rot->info.src_image_h.align[0] = 0;
	fb_rot->info.src_image_h.align[1] = 0;
	fb_rot->info.src_image_h.align[2] = 0;
	fb_rot->info.src_image_h.clip_rect.x = 0;
	fb_rot->info.src_image_h.clip_rect.y = 0;
	fb_rot->info.src_image_h.clip_rect.w = fb_rot->info.src_image_h.width;
	fb_rot->info.src_image_h.clip_rect.h = fb_rot->info.src_image_h.height;
	fb_rot->info.src_image_h.alpha = 1;
	fb_rot->info.src_image_h.mode = 255;
	fb_rot->info.src_image_h.use_phy_addr = 1;


	/*double buffer*/
	fb_rot->dst_mem_len = p_info->fix.line_length * p_info->var.yres * 2;
	fb_rot->dst_vir_addr =
		disp_malloc(fb_rot->dst_mem_len, (u32 *)(&fb_rot->dst_phy_addr));
	if (!fb_rot->dst_vir_addr || !fb_rot->dst_phy_addr)
		goto G2D_RELEASE;
	config->info.fb.addr[0] =
	    (unsigned long long)fb_rot->dst_phy_addr + fb_rot->dst_mem_len / 2;

	fb_rot->info.dst_image_h.laddr[0] = (u32)fb_rot->dst_phy_addr;
	fb_rot->info.dst_image_h.align[0] = 0;
	fb_rot->info.dst_image_h.align[1] = 0;
	fb_rot->info.dst_image_h.align[2] = 0;
	fb_rot->info.dst_image_h.clip_rect.x = 0;
	fb_rot->info.dst_image_h.clip_rect.y = 0;
	fb_rot->info.dst_image_h.alpha = 1;
	fb_rot->info.dst_image_h.mode = 255;
	fb_rot->info.dst_image_h.use_phy_addr = 1;

	fb_rot->fb = p_info;
	fb_rot->switch_buffer_flag = 0;
	fb_rot->apply = fb_g2d_rot_apply;
	fb_rot->free = fb_g2d_rot_free;
	fb_rot->set_degree = fb_g2d_set_degree;


	return fb_rot;

G2D_RELEASE:
	g2d_release(0, &g2d_file);
ERROR:
	kfree(fb_rot);
	return NULL;
}
