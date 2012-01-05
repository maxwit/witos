#include <io.h>
#include <irq.h>
#include <init.h>
#include <board.h>
#include <platform.h>

#ifdef CONFIG_SMSC91X
static int __INIT__ smsc91x_device_init(struct platform_device *pdev)
{
	__u32 val;

	val = readl(VA(GPMC_CONFIG1(1)));
	val &= ~(0x3 << 10);
	val &= ~(0x1 << 9);
	val |= 0x1;
	writel(VA(GPMC_CONFIG1(1)), val);

	val = 0x8 << 8 | 0x1 << 6 | 0x8;
	writel(VA(GPMC_CONFIG7(1)), val);

	return 0;
}

// fixme
static struct platform_device smsc91x_device = {
	.dev = {
		.mem = 0x28000000,
		.irq = GPIO_IRQ(19),
	},
	.name = "SMSC SMSC91X",
	.init = smsc91x_device_init,
};
#endif

// fixme: __INITDATA__
static struct platform_device *beagle_devices[] = {
#ifdef CONFIG_SMSC91X
	&smsc91x_device,
#endif
};

static int __INIT__ beagle_init(struct board_desc *board, const struct board_id *id)
{
	int i, ret;

	for (i = 0; i < ARRAY_ELEM_NUM(beagle_devices); i++) {
		ret = platform_device_register(beagle_devices[i]);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static struct board_id beagle_idt[] = {
	BOARD_ID("beagle", 1546),
	{}
};

BOARD_DESC("beagle", beagle_idt, beagle_init);
