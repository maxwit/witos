#include <irq.h>
#include <init.h>
#include <arm/s3c24x0.h>
#include <flash/flash.h>
#include <sysconf.h>

// fxime: add __INITDATA__
static const struct part_attr mw2440_part_tab[] =
{
	{
		.part_type = PT_BL_GTH,
		.part_size = 1, // 1 block
		.part_name = "g-bios",
	},
	{
		.part_type = PT_BL_GBH,
		.part_size = KB(512),
		.part_name = "g-bios",
	},
	{
		.part_type = PT_BL_GCONF,
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
		.part_type = PT_FS_CRAMFS,
		.part_name = "rootfs",
		.part_size = MB(48),
	},
	{
		.part_type = PT_FS_JFFS2,
		.part_name = "data",
	},
};

static int __INIT__ s3c2440_init(void)
{
	u32 val;

#if 1
	val = readl(VA(S3C24X0_GPBCON));
	val &= ~3;
	val |= 2;
	writel(VA(S3C24X0_GPBCON), val);
#endif

	// LED @ GPE12/13
	val = readl(VA(0x56000040));
	val &= ~0x0f000000;
	val |= 0x05000000;
	writel(VA(0x56000040), val);

	val = readl(VA(0x56000044));
	val |= 0x3000;
	writel(VA(0x56000044), val);

	// LED @ GPF4/5/6/7
	val = readl(VA(0x56000050));
	val &= ~0xff00;
	val |= 0x5500;
	writel(VA(0x56000050), val);

	val = readl(VA(0x56000054));
	val |= 0xf0;
	writel(VA(0x56000054), val);

#ifdef CONFIG_IRQ_SUPPORT
	s3c24x0_interrupt_init();

	// fixme
	irq_set_trigger(IRQ_EINT9, IRQ_TYPE_RISING);
	irq_set_trigger(IRQ_EINT7, IRQ_TYPE_RISING);

	s3c24x0_timer_init();
#endif

	flash_add_part_tab(mw2440_part_tab, ARRAY_ELEM_NUM(mw2440_part_tab));

	return 0;
}

PLAT_INIT(s3c2440_init);

