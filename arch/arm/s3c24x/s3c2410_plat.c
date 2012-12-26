#include <io.h>
#include <irq.h>
#include <init.h>

#define CONFIG_DM9000_IRQ        IRQ_EINT7
#define DM9000_PHYS_BASE         0x20000000

static struct platform_device dm9000_device = {
	.dev = {
		.mem = DM9000_PHYS_BASE,
		.irq = CONFIG_DM9000_IRQ,
	},
	.name = "dm9000",
};

static int __init s3c2410_init(void)
{
#ifdef CONFIG_IRQ_SUPPORT
	s3c24x0_interrupt_init();
	// fixme
	irq_set_trigger(IRQ_EINT9, IRQ_TYPE_RISING);

#ifdef CONFIG_TIMER_SUPPORT
	s3c24x0_timer_init();
#endif
#endif

	platform_device_register(&dm9000_device);

	return 0;
}

plat_init(s3c2410_init);
