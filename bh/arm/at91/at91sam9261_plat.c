#include <arm/at91sam926x.h>

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

	return 0;
}

PLAT_INIT(at91sam9261_init);
