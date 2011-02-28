#include <g-bios.h>
#include <flash/flash.h>
#include <arm/at91sam926x.h>
#include <sysconf.h>

// fxime: add __INITDATA__
static const struct part_attr at91sam9261_part_tab[] =
{
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
		.part_size = MB(6),
	},
	{
		.part_type = PT_FS_JFFS2,
		.part_size = MB(24),
		.part_name = "rootfs"
	},
	{
		.part_type = PT_FS_YAFFS2,
		.part_name = "data"
	},
};


// fixme
#define CONFIG_DM9000_PIO_RESET  PIOC
#define CONFIG_DM9000_PIN_RESET  (1 << 10)

#define CONFIG_DM9000_PIO_IRQ    PIOC
#define CONFIG_DM9000_PIN_IRQ    (1 << 11)


static int __INIT__ at91sam9261_init(void)
{
	writel(VA(AT91SAM926X_PA_PMC + PMC_PCER), 0x11);

	writel(VA(AT91SAM926X_PA_SMC + SMC_SETUP(2)), 2 << 16 | 2);
	writel(VA(AT91SAM926X_PA_SMC + SMC_PULSE(2)), 0x08040804);
	writel(VA(AT91SAM926X_PA_SMC + SMC_CYCLE(2)), 16 << 16 | 16);
	writel(VA(AT91SAM926X_PA_SMC + SMC_MODE(2)), 1 << 16 | 0x1103);

	at91_gpio_conf_output(CONFIG_DM9000_PIO_RESET, CONFIG_DM9000_PIN_RESET, 0);

#ifdef CONFIG_IRQ_SUPPORT
	at91_gpio_conf_input(CONFIG_DM9000_PIO_IRQ, CONFIG_DM9000_PIN_IRQ, 0);
	readl(VA(PIO_BASE(CONFIG_DM9000_PIO_IRQ) + PIO_ISR));

	at91sam926x_interrupt_init();

	at91sam926x_timer_init();
#endif

	sysconf_init_part_tab(at91sam9261_part_tab, ARRAY_ELEM_NUM(at91sam9261_part_tab));

	return 0;
}

PLAT_INIT(at91sam9261_init);
