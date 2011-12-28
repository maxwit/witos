#include <irq.h>
#include <init.h>
#include <spi.h>

#ifdef CONFIG_SPI
static struct spi_master mw61_spi_master[] = {
	{
		.name = "s3c6410_spi0",
		.bus_num = 0,
	},
};

static struct spi_slave mw61_spi_slave[] = {
	{
		.name = "w25x_nor_flash",
	},
};
#endif

static int __INIT__ mw61_init(void)
{
	__u32 val;
	struct spi_slave  *spi_slave;
	struct spi_master *spi_master;

	val = readl(VA(SROM_BASE + SROM_BW));
	val &= ~0xF0;
	val |= 0xD0;
	writel(VA(SROM_BASE + SROM_BW), val);

	val = readl(VA(0x7f0080a0));
	val &= ~(3 << 28);
	val |= 2 << 28;
	writel(VA(0x7f0080a0), val);

	val = readl(VA(0x7f008080));
	val &= ~0xFF;
	val |= 0x11;
	writel(VA(0x7f008080), val);

	val = readl(VA(0x7f008084));
	val |= 0x3;
	writel(VA(0x7f008084), val);

#ifdef CONFIG_IRQ_SUPPORT
	s3c6410_interrupt_init();

#ifdef CONFIG_TIMER_SUPPORT
	s3c6410_timer_init();
#endif
#endif
	// MMC0 GPIO setting
	writel(VA(GPG_BASE + GPCON), 0x2222222);

	//gpio setting, EINT0,1 and EINT7
	val = readl(VA(GPN_CON));
	val &= ~0xffff;
	val |= 2 << 14 | 0xa;
	writel(VA(GPN_CON), val);

	//gpio trigger setting, EINT0,1 and EINT7
	val = readl(VA(EINT0_CON));
	val &= ~0x7;
	val |= 3;
	//set dm9000's interrupt,high level active
	val &= ~(0x7 << 12);
	val |= 0x001 << 12;
	writel(VA(EINT0_CON), val);

	//clean external interrupt
	val = readl(VA(EINT0PEND));
	writel(VA(EINT0PEND), val);

#ifdef CONFIG_SPI
	// master 0
	val = readl(VA(GPC_BASE + GPCCON));
	val &= ~0xFFFF;
	val |= 0x2222;
	writel(VA(GPC_BASE + GPCCON), val);

	set_master_tab(mw61_spi_master, ARRAY_ELEM_NUM(mw61_spi_master));

	set_slave_tab(mw61_spi_slave, ARRAY_ELEM_NUM(mw61_spi_slave));

	spi_slave  = get_spi_slave("w25x_nor_flash");
	spi_master = get_spi_master("s3c6410_spi0");
	list_head_init(&spi_master->slave_list);
	spi_slave_attach(spi_master, spi_slave);
#endif

	return 0;
}

PLAT_INIT(mw61_init);

static void s3c6410_reset(void)
{
	__u32 val;

	val = readl(VA(RST_BASE + RST_STAT));
	val |= 0x1;
	writel(VA(RST_BASE + RST_STAT), val);
}

DECL_RESET(s3c6410_reset);