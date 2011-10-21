#include <irq.h>
#include <init.h>
#include <io.h>
#include <flash/flash.h>
#include <sysconf.h>

// fxime: add __INITDATA__
static const struct part_attr omap3530_part_tab[] =
{
	{
		.part_type = PT_BL_GTH,
		.part_size = KB(512),
		.part_name = "g-bios",
	},
	{
		.part_type = PT_BL_GBH,
		.part_size = MB(2),
		.part_name = "g-bios",
	},
	{
		.part_type = PT_BL_GCONF,
		.part_size = 1, // 1 block
		.part_name = "g-bios",
	},
	{
		.part_type = PT_OS_LINUX,
		.part_size = MB(3),
	},
	{
		.part_type = PT_FS_RAMDISK,
		.part_size = MB(3),
	},
	{
		.part_type = PT_FS_JFFS2,
		.part_size = MB(64),
		.part_name = "rootfs"
	},
	{
		.part_type = PT_FS_YAFFS2,
		.part_size = MB(64),
		.part_name = "data_1"
	},
	{
		.part_type = PT_FS_UBIFS,
		.part_name = "data_2"
	},
};

static int __INIT__ omap3530_init(void)
{
	__u32 val, val4;
#ifdef CONFIG_LAN9220
	__u16 val2;
#endif

	//
	val4 = readl(VA(PADCONF_GPMC_NBE1));
	DPRINT("GPIO62 = 0x%08x\n", val4);
	val4 |= 0xffff0000;
	writel(VA(PADCONF_GPMC_NBE1), val4);

	val4 = readl(VA(PADCONF_GPMC_NBE1));
	DPRINT("GPIO62 = 0x%08x\n", val4);

	flash_add_part_tab(omap3530_part_tab, ARRAY_ELEM_NUM(omap3530_part_tab));

	// GPMC
	for (val4 = 0x7A; val4 <= 0x8C; val4 += 2)
		writew(VA(0x48002000 + val4), 0);

	for (; val4 <= 0xAC; val4 += 2)
		writew(VA(0x48002000 + val4), 1 << 8);

#if defined(CONFIG_BOARD_BEAGLE) && defined(CONFIG_SMSC91X)
	val4 = readl(VA(GPMC_CONFIG1(1)));
	val4 &= ~(0x3 << 10);
	val4 &= ~(0x1 << 9);
	val4 |= 0x1;
	writel(VA(GPMC_CONFIG1(1)), val4);

	val4 = 0x8 << 8 | 0x1 << 6 | 0x8;
	writel(VA(GPMC_CONFIG7(1)), val4);
#elif defined(CONFIG_BOARD_DEVKIT8000) && defined(CONFIG_DM9000)
	writew(VA(0x480020BA), 1 << 4 | 1 << 3 | 0);

	writel(VA(0x6E000060 + 6 * 0x30 + 0x0), 0x00001000);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x04), 0x001e1e00);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x08), 0x00080300);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x0c), 0x1c091c09);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x10), 0x04181f1f);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x14), 0x00000FcF);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x18), 0x00000f6c);
#elif defined(CONFIG_BOARD_EVM3530) && defined(CONFIG_LAN9220)
	writew(VA(0x480020B8), 1 << 4 | 1 << 3 | 0);

	writel(VA(0x6E000060 + 5 * 0x30 + 0x00), 0x00001000);
	writel(VA(0x6E000060 + 5 * 0x30 + 0x04), 0x001e1e01);
	writel(VA(0x6E000060 + 5 * 0x30 + 0x08), 0x00080300);
	writel(VA(0x6E000060 + 5 * 0x30 + 0x0c), 0x1c091c09);
	writel(VA(0x6E000060 + 5 * 0x30 + 0x10), 0x04181f1f);
	writel(VA(0x6E000060 + 5 * 0x30 + 0x14), 0x00000fcf);
	writel(VA(0x6E000060 + 5 * 0x30 + 0x18), 0x00000f6c);

	val2 = readw(VA(PADCONF_GPMC_NWE));
	val2 |= 0x0E00;
	writew(VA(PADCONF_GPMC_NWE), val2);

	val2 = readw(VA(PADCONF_GPMC_NOE));
	val2 |= 0x0E00;
	writew(VA(PADCONF_GPMC_NOE), val2);

	val2 = readw(VA(PADCONF_GPMC_NADV_ALE));
	val2 |= 0x0E00;
	writew(VA(PADCONF_GPMC_NADV_ALE), val2);
#endif

	// enable gpio1's clock(function,interface and auto wakeup clock)
	val = readl(VA(0x48004c00));
	val |= 1 << 3;
	writel(VA(0x48004c00), val);

	val = readl(VA(0x48004c10));
	val |= 1 << 3;
	writel(VA(0x48004c10), val);

	val = readl(VA(0x48004c30));
	val |= 1 << 3;
	writel(VA(0x48004c30), val);

#ifdef CONFIG_IRQ_SUPPORT

	omap3530_irq_init();

	irq_enable();
#endif

	return 0;
}

PLAT_INIT(omap3530_init);
