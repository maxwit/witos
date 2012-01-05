#include <io.h>
#include <irq.h>
#include <init.h>
#include <board.h>
#include <platform.h>

#ifdef CONFIG_DM9000
static int __INIT__ dm9000_device_init(struct platform_device *pdev)
{
	writew(VA(0x480020BA), 1 << 4 | 1 << 3 | 0);

	writel(VA(0x6E000060 + 6 * 0x30 + 0x0), 0x00001000);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x04), 0x001e1e00);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x08), 0x00080300);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x0c), 0x1c091c09);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x10), 0x04181f1f);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x14), 0x00000FcF);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x18), 0x00000f6c);

	return 0;
}

static struct platform_device dm9000_device = {
	.dev = {
		.mem = 0x28000000,
		.irq = GPIO_IRQ(19),
	},
	.name = "dm9000",
	.init = dm9000_device_init,
};
#endif

// fixme: __INITDATA__
static struct platform_device *devkit8000_devices[] = {
#ifdef CONFIG_DM9000
	&dm9000_device,
#endif
};

static int __INIT__ devkit8000_init(struct board_desc *board, const struct board_id *id)
{
	int i, ret;

	for (i = 0; i < ARRAY_ELEM_NUM(devkit8000_devices); i++) {
		ret = platform_device_register(devkit8000_devices[i]);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static struct board_id devkit8000_idt[] = {
	BOARD_ID("DevKit8000", 2330),
	{}
};

BOARD_DESC("devkit8000", devkit8000_idt, devkit8000_init);
