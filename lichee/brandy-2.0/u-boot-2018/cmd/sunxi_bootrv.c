// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <environment.h>
#include <errno.h>
#include <malloc.h>
#include <nand.h>
#include <asm/byteorder.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <sunxi_image_verifier.h>
#include <fdt_support.h>
#include <sunxi_board.h>
/*******************************************************************/
/* bootr - boot application image from image in memory */
/*******************************************************************/

int do_bootrv(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	__attribute__((unused)) u32 run_addr = 0;
	__attribute__((unused)) u32 id = 0;
	__attribute__((unused)) u32 img_addr = 0;

#ifdef CONFIG_BOOT_RISCV
	img_addr = simple_strtoul(argv[1], NULL, 16);
	run_addr = simple_strtoul(argv[2], NULL, 16);
	id = simple_strtoul(argv[3], NULL, 16);
	sunxi_riscv_init(img_addr, run_addr, id);
#else
	printf("Don't Support Any Riscv PlatForm.\n");
#endif
	return  0;
}

#ifdef CONFIG_SYS_LONGHELP
static char bootrv_help_text[] =
	"[addr [arg ...]]\n    - boot application image stored in memory\n"
	"\tpassing arguments 'arg ...'; when booting a rtos image,\n"
	"\t'arg[1]' can be the loader address of image\n"
	"\t'arg[2]' can be the run address of image\n"
	"\t'arg[3]' can be cpu id of the ip\n";
#endif

U_BOOT_CMD(
	bootrv,	CONFIG_SYS_MAXARGS,	1,	do_bootrv,
	"boot riscv image from memory", bootrv_help_text
);
