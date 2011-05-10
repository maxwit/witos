#include <irq.h>
#include <init.h>
#include <io.h>
#include <flash/flash.h>
#include <sysconf.h>
#include <spi.h>

// fxime: add __INITDATA__
static const struct part_attr mw61_part_tab[] =
{
	{
		.part_type = PT_BL_GTH,
		.part_size = 1, // 1 block
		.part_name = "g-bios",
	},
	{
		.part_type = PT_BL_GBH,
		.part_size = KB(256),
		.part_name = "g-bios",
	},
	{
		.part_type = PT_BL_GCONF,
		.part_size = 1, // 1 block
		.part_name = "g-bios",
	},
	{
		.part_type = PT_OS_LINUX,
		.part_size = MB(2),
	},
	{
		.part_type = PT_FS_RAMDISK,
		.part_size = MB(2),
	},
	{
		.part_type = PT_FS_JFFS2,
		.part_size = MB(64),
		.part_name = "rootfs"
	},
	{
		.part_type = PT_FS_YAFFS2,
		.part_size = MB(64),
		.part_name = "data_1"
	},
	{
		.part_type = PT_FS_UBIFS,
		.part_name = "data_2"
	},
};

#ifdef CONFIG_SPI
static struct spi_master mw61_spi_master[] =
{
	{
		.name = "s3c6410_spi0",
		.bus_num = 0,
	},
};

static struct spi_slave mw61_spi_slave[] =
{
	{
		.name = "w25x_nor_flash",
	},
};
#endif

static int __INIT__ mw61_init(void)
{
	u32 val;
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

	s3c6410_timer_init();
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

	flash_add_part_tab(mw61_part_tab, ARRAY_ELEM_NUM(mw61_part_tab));

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
