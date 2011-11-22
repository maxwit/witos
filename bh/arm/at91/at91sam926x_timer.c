#include <irq.h>
#include <arm/at91sam926x.h>

static int at91sam926x_pit_isr(__u32 irq, void *pDev)
{
	if (!(readl(VA(AT91SAM926X_PA_PITC + PITC_SR)) & 0x1))
		return IRQ_NONE;

	inc_tick();

	readl(VA(AT91SAM926X_PA_PITC + PITC_PIVR));

	return IRQ_HANDLED;
}

int __INIT__ at91sam926x_timer_init(void)
{
	int ret;

	writel(VA(AT91SAM926X_PA_PITC + PITC_MR), (MCK_RATE / 16 / 1000));

	ret = irq_register_isr(1, at91sam926x_pit_isr, NULL);
	if (ret < 0) {
		printf("%s(): irq_register_isr() failed!\n", __func__);
		return ret;
	}

	writel(VA(AT91SAM926X_PA_PITC + PITC_MR), (0x3 << 24) | readl(VA(AT91SAM926X_PA_PITC + PITC_MR)));

	calibrate_delay(1000);

	return ret;
}
