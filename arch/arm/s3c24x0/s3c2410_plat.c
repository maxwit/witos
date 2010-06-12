#include <g-bios.h>
#include <irq.h>
#include <flash/flash.h>
#include <sysconf.h>

// fxime: add __INITDATA__
static const struct part_attr mw2410_part_tab[] =
{
	{
		.part_type  = PT_BL_GTH,
		.part_size = 1, // 1 block
		.part_name = "g-bios",
	},
	{
		.part_type  = PT_BL_GBH,
		.part_size = KB(512),
		.part_name = "g-bios",
	},
	{
		.part_type	= PT_BL_GB_CONF,
		.part_size = 1, // 1 block
		.part_name = "sysconfig",
	},
	{
		.part_type  = PT_OS_LINUX,
		.part_size = MB(2),
	},
	{
		.part_type	= PT_FS_RAMDISK,
		.part_size = MB(6),
	},
	{
		.part_type	= PT_FS_JFFS2,
		.part_size = MB(24),
		.part_name = "rootfs"
	},
	{
		.part_type	= PT_FS_YAFFS2,
		.part_name = "data"
	},
};


static int __INIT__ s3c2410_init(void)
{
#ifdef CONFIG_IRQ_SUPPORT
	s3c24x0_interrupt_init();
	// fixme
	irq_set_trigger(IRQ_EINT9, IRQ_TYPE_RISING);

	s3c24x0_timer_init();
#endif

	sysconf_init_part_tab(mw2410_part_tab, ARRAY_ELEM_NUM(mw2410_part_tab));

	return 0;
}

PLAT_INIT(s3c2410_init);

