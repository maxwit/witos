#include <irq.h>
#include <init.h>
#include <arm/s3c24x0.h>

static int __INIT__ s3c2440_init(void)
{
	__u32 val;

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

	return 0;
}

PLAT_INIT(s3c2440_init);

