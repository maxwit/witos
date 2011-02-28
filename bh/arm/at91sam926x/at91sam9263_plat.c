#include <g-bios.h>
#include <flash/flash.h>
#include <arm/at91sam926x.h>
#include <sysconf.h>
#define CONFIG_PIO_PC10 (1 << 10)


// fxime: add __INITDATA__
const struct part_attr at91sam9263_part_tab[] =
{
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
		.part_size = MB(2),
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


static int __INIT__ at91sam9263_init(void)
{
	u32 mask;

	at91_gpio_conf_input(PIOE, 1 << 31, 0);

	at91_gpio_conf_periB(PIOC, 1 << 25, 0);	// ERXDV

	mask = 1 << 21 | // ETXCK_EREFCK
		1 << 23 |  // ETX0
		1 << 24 |  // ETX1
		1 << 25 |  // ERX0
		1 << 26 |  // ERX1
		1 << 27 |  // ERXER
		1 << 28 |  // ETXEN
		1 << 29 |  // EMDC
		1 << 30;   // EMDIO
	at91_gpio_conf_periA(PIOE, mask, 0);

	// fixme
	at91_rstc_writel(RSTC_MR, (0xA5 << 24) | (0x0d << 8) | 0x1);
	at91_rstc_writel(RSTC_CR, (0xA5 << 24) | (0x1 << 3));

	while (!(at91_rstc_readl(RSTC_SR) & (0x1 << 16)));

#ifdef CONFIG_IRQ_SUPPORT
	at91sam926x_interrupt_init();

	at91sam926x_timer_init();
#endif

	sysconf_init_part_tab(at91sam9263_part_tab, ARRAY_ELEM_NUM(at91sam9263_part_tab));

	return 0;
}

PLAT_INIT(at91sam9263_init);
