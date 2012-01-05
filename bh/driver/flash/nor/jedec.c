#include <spi.h>

#define JEDEC_READ_ID   0x9F

struct jedec_id {
	__u8 vendor_id;
	__u8 device_id;
};

static const struct jedec_id nor_ids[] =
{
	{0xEF, 0x13}, {0xEF, 0x12}, {0xEF, 0x11}, {0xEF, 0x10}
};

static int read_jedec_id(struct spi_slave *spi, __u8 *rx_buf)
{
	__u8 tx_buf[1] = {JEDEC_READ_ID};

	spi_write_then_read(spi, tx_buf, 1, rx_buf, 3 /* 5? */);
	printf("\n-------------------\n\n");
//	spi_write_then_read(spi, tx_buf, 1, rx_buf, 3 /* 5? */);
//	printf("\n-------------------\n\n");
//	spi_write_then_read(spi, tx_buf, 1, rx_buf, 3 /* 5? */);

	return 0;
}

static int __INIT__ nor_flash_probe(void)
{
	int i;
	__u8 jedec_id[64];
	struct spi_slave *nor_flash;

	nor_flash = get_spi_slave("w25x_nor_flash");
	read_jedec_id(nor_flash, jedec_id);

	for (i = 0; i < ARRAY_ELEM_NUM(nor_ids); i++) {
		if (jedec_id[0] == nor_ids[i].vendor_id) {
			printf("SPI Nor flash detected! flash details:\n"
				"\tmanufacturer ID = 0x%02x\n"
				"\tdevice ID       = 0x%02x, 0x%02x\n",
				jedec_id[0], jedec_id[1], jedec_id[2]);

			return 0;
		}
	}

	printf("SPI Nor Flash not detected!\n");

	return -ENODEV;
}

module_init(nor_flash_probe);
