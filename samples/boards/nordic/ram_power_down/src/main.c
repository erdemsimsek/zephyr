/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/util.h>
#include <stdio.h>
#include <stdint.h>

#include "tfm_ioctl_core_api.h"

#if !defined(CONFIG_NRF_TFM_RAM_CTRL_SERVICE)
#error "This sample needs the TF-M RAM-control service (build for a *_ns target)."
#endif

/*
 * MEMCONF controls RAM power in 32 KiB sections on nRF54L / nRF7120. Round the
 * used-RAM end up to this granularity so the (partially used) section holding
 * live data is left powered.
 */
#define RAM_SECTION_SIZE 0x8000U

/* End of the RAM actually occupied by this image (Zephyr linker symbol). */
extern char _image_ram_end[];

/* The non-secure RAM region granted to this image. */
#define SRAM_NODE DT_CHOSEN(zephyr_sram)
#define SRAM_BASE ((uintptr_t)DT_REG_ADDR(SRAM_NODE))
#define SRAM_END  ((uintptr_t)(DT_REG_ADDR(SRAM_NODE) + DT_REG_SIZE(SRAM_NODE)))

static void dump_control(const char *when)
{
	uint32_t control = 0;
	enum tfm_platform_err_t err =
		tfm_platform_ram_ctrl_dump(&control, NULL, NULL, NULL);

	if (err != TFM_PLATFORM_ERR_SUCCESS) {
		printf("MEMCONF dump failed (err=%d)\n", (int)err);
		return;
	}

	printf("MEMCONF CONTROL %s: 0x%08x\n", when, control);
}

int main(void)
{
	uintptr_t used_end = (uintptr_t)_image_ram_end;
	uintptr_t unused_start = ROUND_UP(used_end, RAM_SECTION_SIZE);
	uintptr_t unused_end = SRAM_END;

	printf("\n%s RAM power-down demo\n", CONFIG_BOARD);
	printf("NS RAM 0x%08lx..0x%08lx, image uses up to 0x%08lx\n",
	       (unsigned long)SRAM_BASE, (unsigned long)unused_end,
	       (unsigned long)used_end);

	/* A bit in CONTROL is 1 when its 32 KiB section is powered. */
	dump_control("before");

	if (unused_start < unused_end) {
		size_t len = unused_end - unused_start;
		enum tfm_platform_err_t err = tfm_platform_ram_ctrl_power_set(
			(uint32_t)unused_start, (uint32_t)len, false);

		printf("Power down 0x%08lx..0x%08lx (%u KiB): %s\n",
		       (unsigned long)unused_start, (unsigned long)unused_end,
		       (unsigned int)(len / 1024U),
		       err == TFM_PLATFORM_ERR_SUCCESS ? "ok" : "FAILED");
	} else {
		printf("No spare RAM section above the image to power down\n");
	}

	/* Cleared bits are the sections that have just been powered off. */
	dump_control("after");
	printf("Running with unused RAM powered off.\n");
	return 0;
}
