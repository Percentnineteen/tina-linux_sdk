// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2017 The Android Open Source Project
 */
#include <common.h>
#include <android_ab.h>
#include <android_bootloader_message.h>
#include <malloc.h>
#include <linux/err.h>
#include <memalign.h>
#include <u-boot/crc.h>
#include <u-boot/crc.h>
#include <securestorage.h>

int sunxi_flash_try_partition(struct blk_desc *desc, const char *str,
			      disk_partition_t *info);

/**
 * Compute the CRC-32 of the bootloader control struct.
 *
 * Only the bytes up to the crc32_le field are considered for the CRC-32
 * calculation.
 *
 * @param[in] abc bootloader control block
 *
 * @return crc32 sum
 */
static uint32_t ab_control_compute_crc(struct bootloader_control *abc)
{
	return crc32(0, (void *)abc, offsetof(typeof(*abc), crc32_le));
}

/**
 * Initialize bootloader_control to the default value.
 *
 * It allows us to boot all slots in order from the first one. This value
 * should be used when the bootloader message is corrupted, but not when
 * a valid message indicates that all slots are unbootable.
 *
 * @param[in] abc bootloader control block
 *
 * @return 0 on success and a negative on error
 */
static int ab_control_default(struct bootloader_control *abc)
{
	int i;
	const struct slot_metadata metadata = {.priority	 = 15,
					       .tries_remaining  = 7,
					       .successful_boot  = 0,
					       .verity_corrupted = 0,
					       .reserved	 = 0 };

	if (!abc)
		return -EFAULT;

	memcpy(abc->slot_suffix, "a\0\0\0", 4);
	abc->magic   = BOOT_CTRL_MAGIC;
	abc->version = BOOT_CTRL_VERSION;
	abc->nb_slot = NUM_SLOTS;
	memset(abc->reserved0, 0, sizeof(abc->reserved0));
	for (i			  = 0; i < abc->nb_slot; ++i)
		abc->slot_info[i] = metadata;

	memset(abc->reserved1, 0, sizeof(abc->reserved1));
	abc->crc32_le = ab_control_compute_crc(abc);

	return 0;
}

/**
 * Load the boot_control struct from disk into newly allocated memory.
 *
 * This function allocates and returns an integer number of disk blocks,
 * based on the block size of the passed device to help performing a
 * read-modify-write operation on the boot_control struct.
 * The boot_control struct offset (2 KiB) must be a multiple of the device
 * block size, for simplicity.
 *
 * @param[in] dev_desc Device where to read the boot_control struct from
 * @param[in] part_info Partition in 'dev_desc' where to read from, normally
 *			the "misc" partition should be used
 * @param[out] pointer to pointer to bootloader_control data
 * @return 0 on success and a negative on error
 */
static int ab_control_create_from_disk(struct blk_desc *dev_desc,
				       const disk_partition_t *part_info,
				       struct bootloader_control **abc)
{
	ulong abc_offset, abc_blocks, ret;

	abc_offset = offsetof(struct bootloader_message_ab, slot_suffix);
	if (abc_offset % part_info->blksz) {
		pr_err("ANDROID: Boot control block not block aligned.\n");
		return -EINVAL;
	}
	abc_offset /= part_info->blksz;

	abc_blocks = DIV_ROUND_UP(sizeof(struct bootloader_control),
				  part_info->blksz);
	if (abc_offset + abc_blocks > part_info->size) {
		pr_err("ANDROID: boot control partition too small. Need at");
		pr_err(" least %lu blocks but have %lu blocks.\n",
		       abc_offset + abc_blocks, part_info->size);
		return -EINVAL;
	}
	*abc = malloc_cache_aligned(abc_blocks * part_info->blksz);
	if (!*abc)
		return -ENOMEM;

	ret = blk_dread(dev_desc, part_info->start + abc_offset, abc_blocks,
			*abc);
	if (IS_ERR_VALUE(ret)) {
		pr_err("ANDROID: Could not read from boot ctrl partition\n");
		free(*abc);
		return -EIO;
	}

	pr_msg("ANDROID: Loaded ABC, %lu blocks\n", abc_blocks);

	return 0;
}

/**
 * Store the loaded boot_control block.
 *
 * Store back to the same location it was read from with
 * ab_control_create_from_misc().
 *
 * @param[in] dev_desc Device where we should write the boot_control struct
 * @param[in] part_info Partition on the 'dev_desc' where to write
 * @param[in] abc Pointer to the boot control struct and the extra bytes after
 *                it up to the nearest block boundary
 * @return 0 on success and a negative on error
 */
static int ab_control_store(struct blk_desc *dev_desc,
			    const disk_partition_t *part_info,
			    struct bootloader_control *abc)
{
	ulong abc_offset, abc_blocks, ret;

	abc_offset = offsetof(struct bootloader_message_ab, slot_suffix) /
		     part_info->blksz;
	abc_blocks = DIV_ROUND_UP(sizeof(struct bootloader_control),
				  part_info->blksz);
	ret = blk_dwrite(dev_desc, part_info->start + abc_offset, abc_blocks,
			 abc);
	if (IS_ERR_VALUE(ret)) {
		pr_err("ANDROID: Could not write back the misc partition\n");
		return -EIO;
	}

	return 0;
}

/**
 * Compare two slots.
 *
 * The function determines slot which is should we boot from among the two.
 *
 * @param[in] a The first bootable slot metadata
 * @param[in] b The second bootable slot metadata
 * @return Negative if the slot "a" is better, positive of the slot "b" is
 *         better or 0 if they are equally good.
 */
static int ab_compare_slots(const struct slot_metadata *a,
			    const struct slot_metadata *b)
{
	/* Higher priority is better */
	if (a->priority != b->priority)
		return b->priority - a->priority;

	/* Higher successful_boot value is better, in case of same priority */
	if (a->successful_boot != b->successful_boot)
		return b->successful_boot - a->successful_boot;

	/* Higher tries_remaining is better to ensure round-robin */
	if (a->tries_remaining != b->tries_remaining)
		return b->tries_remaining - a->tries_remaining;

	return 0;
}

int ab_select_slot(struct blk_desc *dev_desc, disk_partition_t *part_info)
{
	struct bootloader_control *abc = NULL;
	u32 crc32_le = 0;
	int slot, i, ret;
	bool store_needed = false;
	char slot_suffix[4];

	ret = ab_control_create_from_disk(dev_desc, part_info, &abc);
	if (ret < 0) {
		/*
		 * This condition represents an actual problem with the code or
		 * the board setup, like an invalid partition information.
		 * Signal a repair mode and do not try to boot from either slot.
		 */
		return ret;
	}

#ifdef CONFIG_SUNXI_SECURE_STORAGE
	char boot_slot		= 0;
	uint8_t slot_b		= 0;
	uint8_t kActivePriority = 15;
	uint8_t kActiveTries    = 6;

	ret = sunxi_secure_storage_write_or_read("set-active-boot-slot",
						 &boot_slot, 1, 1);

	if ((ret >= 0) && (boot_slot > 0)) {
		slot_b = boot_slot - 0x31;

		for (i = 0; i < abc->nb_slot; ++i) {
			if (i != slot_b) {
				if (abc->slot_info[i].priority >=
				    kActivePriority) {
					abc->slot_info[i].priority =
						kActivePriority - 1;
				}
			}
		}
		abc->slot_info[slot_b].priority	= kActivePriority;
		abc->slot_info[slot_b].tries_remaining = kActiveTries;
		abc->crc32_le = ab_control_compute_crc(abc);
		boot_slot     = 0;
		sunxi_secure_storage_write_or_read("set-active-boot-slot",
						   &boot_slot, 1, 0);
	}

#endif
	crc32_le = ab_control_compute_crc(abc);

	if (abc->crc32_le != crc32_le) {
		pr_err("ANDROID: Invalid CRC-32 (expected %.8x, found %.8x),",
		       crc32_le, abc->crc32_le);
		pr_err("re-initializing A/B metadata.\n");

		ret = ab_control_default(abc);
		if (ret < 0) {
			free(abc);
			return -ENODATA;
		}
		store_needed = true;
	}

	if (abc->magic != BOOT_CTRL_MAGIC) {
		pr_err("ANDROID: Unknown A/B metadata: %.8x\n", abc->magic);
		free(abc);
		return -ENODATA;
	}

	if (abc->version > BOOT_CTRL_VERSION) {
		pr_err("ANDROID: Unsupported A/B metadata version: %.8x\n",
		       abc->version);
		free(abc);
		return -ENODATA;
	}

	/*
	 * At this point a valid boot control metadata is stored in abc,
	 * followed by other reserved data in the same block. We select a with
	 * the higher priority slot that
	 *  - is not marked as corrupted and
	 *  - either has tries_remaining > 0 or successful_boot is true.
	 * If the selected slot has a false successful_boot, we also decrement
	 * the tries_remaining until it eventually becomes unbootable because
	 * tries_remaining reaches 0. This mechanism produces a bootloader
	 * induced rollback, typically right after a failed update.
	 */

	/* Safety check: limit the number of slots. */
	if (abc->nb_slot > ARRAY_SIZE(abc->slot_info)) {
		abc->nb_slot = ARRAY_SIZE(abc->slot_info);
		store_needed = true;
	}

	slot = -1;
	for (i = 0; i < abc->nb_slot; ++i) {
		if (abc->slot_info[i].verity_corrupted ||
		    !abc->slot_info[i].tries_remaining) {
			pr_msg("ANDROID: unbootable slot %d tries: %d, ", i,
			       abc->slot_info[i].tries_remaining);
			pr_msg("corrupt: %d\n",
			       abc->slot_info[i].verity_corrupted);
			continue;
		}
		pr_msg("ANDROID: bootable slot %d pri: %d, tries: %d, ", i,
		       abc->slot_info[i].priority,
		       abc->slot_info[i].tries_remaining);
		pr_msg("corrupt: %d, successful: %d\n",
		       abc->slot_info[i].verity_corrupted,
		       abc->slot_info[i].successful_boot);

		if (slot < 0 ||
		    ab_compare_slots(&abc->slot_info[i],
				     &abc->slot_info[slot]) < 0) {
			slot = i;
		}
	}

	if (slot >= 0 && !abc->slot_info[slot].successful_boot) {
		pr_err("ANDROID: Attempting slot %c, tries remaining %d\n",
		       BOOT_SLOT_NAME(slot),
		       abc->slot_info[slot].tries_remaining);
		abc->slot_info[slot].tries_remaining--;
		store_needed = true;
	}

	if (slot >= 0) {
		/*
		 * Legacy user-space requires this field to be set in the BCB.
		 * Newer releases load this slot suffix from the command line
		 * or the device tree.
		 */
		memset(slot_suffix, 0, sizeof(slot_suffix));
		slot_suffix[0] = BOOT_SLOT_NAME(slot);
		if (memcmp(abc->slot_suffix, slot_suffix,
			   sizeof(slot_suffix))) {
			memcpy(abc->slot_suffix, slot_suffix,
			       sizeof(slot_suffix));
			store_needed = true;
		}
	}

	if (store_needed) {
		abc->crc32_le = ab_control_compute_crc(abc);
		ab_control_store(dev_desc, part_info, abc);
	}
	free(abc);

	if (slot < 0)
		return -EINVAL;

	return slot;
}

int ab_select_slot_from_partname(char *part_name)
{
	static struct blk_desc *dev_desc;
	static disk_partition_t part_info;
	static int ret = -37;
	int i;

	if (ret == -37) {
		if ((dev_desc == NULL) || (strncmp(part_name, (char *)part_info.name, strlen(part_name)))) {
			dev_desc = blk_get_devnum_by_typename("sunxi_flash", 0);
			if (dev_desc == NULL) {
				debug("%s: get desc fail\n", __func__);
				return CMD_RET_FAILURE;
			}

			for (i = 1;; i++) {
				ret = part_get_info(dev_desc, i, &part_info);
				debug("%s: try part %d, ret = %d\n", __func__, i, ret);
				if (ret < 0)
					return ret;

				if (!strncmp((const char *)part_info.name, part_name,
					     sizeof(part_info.name)))
					break;
			}

		}
		ret = ab_select_slot(dev_desc, &part_info);
	}
	return ret;
}

static int do_enable_sunxi_ab_test(cmd_tbl_t *cmdtp, int flag, int argc,
				   char *const argv[])
{
	struct bootloader_control *abc = NULL;
	int i, ret;
	char part_name[] = "misc";
	static struct blk_desc *dev_desc;
	static disk_partition_t part_info;
	const struct slot_metadata metadata = { .priority	  = 15,
						.tries_remaining  = 7,
						.successful_boot  = 0,
						.verity_corrupted = 0,
						.reserved	  = 0 };

	dev_desc = blk_get_devnum_by_typename("sunxi_flash", 0);
	if (dev_desc == NULL) {
		debug("%s: get desc fail\n", __func__);
		return CMD_RET_FAILURE;
	}

	for (i = 1;; i++) {
		ret = part_get_info(dev_desc, i, &part_info);
		debug("%s: try part %d, ret = %d\n", __func__, i, ret);
		if (ret < 0)
			return ret;

		if (!strncmp((const char *)part_info.name, part_name,
			     sizeof(part_info.name)))
			break;
	}

	ret = ab_control_create_from_disk(dev_desc, &part_info, &abc);
	if (ret < 0) {
		/*
		 * This condition represents an actual problem with the code or
		 * the board setup, like an invalid partition information.
		 * Signal a repair mode and do not try to boot from either slot.
		 */
		return ret;
	}

	pr_err("creating default ab control info\n");
	ret = ab_control_default(abc);
	if (ret < 0) {
		free(abc);
		return -ENODATA;
	}
	//0 and 1 created from default share same const struct
	//give slot 1 a different value
	abc->slot_info[1]	   = metadata;
	abc->slot_info[1].priority = 12;

	abc->crc32_le = ab_control_compute_crc(abc);
	ab_control_store(dev_desc, &part_info, abc);

	pr_err("reset bootcmd to \"reset\" for reboot test\n");
	run_command("env set bootcmd reset", 0);
	run_command("env save", 0);

	tick_printf("prepare ab test done, reset to start the test\n");

	return 0;
}
U_BOOT_CMD(
	sunxi_ab_test, 4, 0, do_enable_sunxi_ab_test,
	"enable test for uboot ab select feature",
	""
	);
