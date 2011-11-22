#pragma once

struct spi_slave;

struct spi_master {
	char *name;
	int bus_num;
	struct list_node slave_list;
	int (*transfer)(struct spi_slave *);
};

struct spi_slave {
	char *name;
	struct list_node slave_node;
	struct spi_master *master;
	struct list_node msg_qu;
};

struct spi_trans_msg {
	struct list_node msg_node;
	__u8 *tx_buf;
	__u8 *rx_buf;
	__u32 len;
};

struct spi_master *spi_master_alloc();

struct spi_slave  *spi_slave_alloc();

int spi_slave_attach(struct spi_master *, struct spi_slave*);

int spi_master_register(struct spi_master *);

int spi_write_then_read(struct spi_slave *slave, __u8 *tx_buf, __u32 n_tx, __u8 *rx_buf, __u32 n_rx);

int spi_transfer(struct spi_slave *slave);

struct spi_slave *get_spi_slave(char *name);

struct spi_master *get_spi_master(char *name);

int set_master_tab(struct spi_master spi_master[], int len);
int set_slave_tab(struct spi_slave spi_slave[], int len);
