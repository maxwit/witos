#include <io.h>
#include <irq.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>

int read_irq_num(void)
{
	int irq_num;
	__u32 val, shift;

	irq_num = readl(VA(INTCPS_BASE + INTCPS_SIR_IRQ)) & 0x7f;

	if (irq_num >= 29 && irq_num <= 34) {
		val = readl(VA(GPIO_IRQ_STATUS1(irq_num - 29)));

		shift = 0;
		while (shift < 32) {
			if ((0x1UL << shift) & val)
				break;

			shift++;
		}

		irq_num = INTC_PINS + shift;
	}

	return irq_num;
}

static void omap3_irq_umask(__u32 irq_num)
{
	switch (irq_num) {
	case 20:
		// GPMC INT
		break;

	case GPIO_IRQ(0) ... GPIO_IRQ(191):
		irq_num -= INTC_PINS;
		writel(VA(GPIO_IRQ_SETENABLE1(irq_num >> 5)), 0x1 << (irq_num & 0x1f));
		irq_num = 29 + (irq_num >> 5);
		break;

		// other condition
	default:
		break;
	}

	writel(VA(INTCPS_BASE + INTCPS_MIRN_CLEN(irq_num >> 5)), 1 << (irq_num & 0x1f));
}

static void omap3_irq_mask(__u32 irq)
{
	writel(VA(INTCPS_BASE + INTCPS_MIRN_SETN(irq >> 5)), 1 << (irq & 0x1f));
}

static int omap3_set_trigger(__u32 nr, __u32 type)
{
	int off;

	if (nr < INTC_PINS)
		return -EINVAL;

	off = (nr - INTC_PINS) & 0x1F;

	switch (type) {
	case IRQ_TYPE_LOW:
		writel(VA(GPIO1_BASE + LEVELDETECT1), 1 << off);
		break;

	case IRQ_TYPE_HIGH:
		writel(VA(GPIO1_BASE + LEVELDETECT0), 1 << off);
		break;

	case IRQ_TYPE_FALLING:
		break;

	case IRQ_TYPE_RISING:
		break;

	default:
		break;
	}

	return 0;
}

static struct int_ctrl omap3_intctl = {
	.mask  = omap3_irq_mask,
	.umask = omap3_irq_umask,
	.set_trigger = omap3_set_trigger,
};

static int handle_dev_irq_list(__u32 irq, struct irq_dev *idev)
{
	int retval = IRQ_NONE;

	do {
		int ret;

		ret = idev->dev_isr(irq, idev->device);
		retval |= ret;

		idev = idev->next;
	} while (idev);

	return retval;
}

static void omap3_irq_handle(struct int_pin *pin, __u32 irq)
{
	__u32 irq_status;
	struct irq_dev *dev_list = pin->dev_list;

	assert(dev_list);

	handle_dev_irq_list(irq, dev_list);

	switch (irq) {
	case 20:
		// GPMC INT
		irq_status = readl(VA(GPMC_BASE + GPMC_IRQ_STATUS));
		writel(VA(GPMC_BASE + GPMC_IRQ_STATUS), irq_status);
		break;

	case GPIO_IRQ(0) ...  GPIO_IRQ(191):
		irq = (irq - INTC_PINS) >> 5;
		irq_status = readl(VA(GPIO_IRQ_STATUS1(irq)));
		writel(VA(GPIO_IRQ_STATUS1(irq)), irq_status);
		break;

	default:
		break;
	}

	writel(VA(INTCPS_BASE + INTCPS_CONTROL), 0x1);
}

int omap3_irq_init(void)
{
	int irq_num;

	// reset the INTC
	writel(VA(INTCPS_BASE + INTCPS_SYSCONFIG), 0x1 << 1);
	while (readl(VA(INTCPS_BASE + INTCPS_SYSCONFIG)) & (0x1 << 1));

	writel(VA(INTCPS_BASE + INTCPS_SYSCONFIG), 0x1);

	for (irq_num = 0; irq_num < MAX_IRQ_NUM; irq_num++) {
		irq_assoc_intctl(irq_num, &omap3_intctl);
		irq_set_handler(irq_num, omap3_irq_handle, 0);

		if (irq_num < INTC_PINS)
			writel(VA(INTCPS_BASE + INTCPS_ILRM(irq_num)), 0x0);
	}

	return 0;
}
