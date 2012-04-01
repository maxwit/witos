#include <io.h>
#include <irq.h>
#include <init.h>

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

	return 0;
}

plat_init(s3c2410_init);
