#include <irq.h>
#include <init.h>
#include <io.h>
#include <arm/s3c6410_reg.h>
#include <flash/flash.h>
#include <sysconf.h>


// fxime: add __INITDATA__
static const struct part_attr smdk6410_part_tab[] =
{
#if 0
	{
		.part_type = PT_BL_GTH,
		.part_size = 1,
		.part_name = "g-bios",
	},
#endif
	{
		.part_type = PT_BL_GBH,
		.part_size = KB(512),
		.part_name = "g-bios",
	},
	{
		.part_type = PT_BL_GB_CONF,
		.part_size = 1, // 1 block
		.part_name = "sysconfig",
	},
	{
		.part_type = PT_OS_LINUX,
		.part_size = MB(2),
	},
	{
		.part_type = PT_FS_RAMDISK,
		.part_size = MB(2),
	},
	{
		.part_type = PT_FS_JFFS2,
		.part_size = MB(64),
		.part_name = "rootfs"
	},
	{
		.part_type = PT_FS_YAFFS2,
		.part_name = "data"
	},
};


static int __INIT__ s3c6410_init(void)
{
	u32 val;

	val = readl(VA(SROM_BASE + SROM_BW));
	val &= ~0xF0;
	val |= 0xD0;
	writel(VA(SROM_BASE + SROM_BW), val);

	val = readl(VA(0x7f0080a0));
	val &= ~(3 << 28);
	val |= 2 << 28;
	writel(VA(0x7f0080a0), val);

	val = readl(VA(0x7f008080));
	val &= ~0xFF;
	val |= 0x11;
	writel(VA(0x7f008080), val);

	val = readl(VA(0x7f008084));
	val |= 0x3;
	writel(VA(0x7f008084), val);

#ifdef CONFIG_IRQ_SUPPORT
	s3c6410_interrupt_init();

	s3c6410_timer_init();
#endif

	sysconf_init_part_tab(smdk6410_part_tab, ARRAY_ELEM_NUM(smdk6410_part_tab));

	return 0;
}

PLAT_INIT(s3c6410_init);
