#include <spi.h>

#define MAX_SPI 10

static struct spi_master *g_spi_masters[MAX_SPI];

int spi_transfer(struct spi_slave *slave)
{
	// TODO: add checking code here
	struct spi_master *master = slave->master;

	return master->transfer(slave);
}

int spi_write_then_read(struct spi_slave *slave, __u8 *tx_buf, __u32 n_tx, __u8 *rx_buf, __u32 n_rx)
{
	enum TX_TYPE {TX, RX};
	struct spi_trans_msg tx[2];

	tx[TX].tx_buf = tx_buf;
	tx[TX].rx_buf = NULL;
	tx[TX].len= n_tx;

	tx[RX].tx_buf = NULL;
	tx[RX].rx_buf = rx_buf;
	tx[RX].len   = n_rx;

	list_head_init(&slave->msg_qu);
	list_add_tail(&tx[TX].msg_node, &slave->msg_qu);
	list_add_tail(&tx[RX].msg_node, &slave->msg_qu);

	return spi_transfer(slave);
}

struct spi_master *spi_master_alloc()
{
	struct spi_master *master;

	master = (struct spi_master *)malloc(sizeof(*master));
	if (NULL == master)
		return NULL;

	master->bus_num = -1;
	list_head_init(&master->slave_list);
	master->transfer = NULL;

	return master;
}

struct spi_slave  *spi_slave_alloc()
{
	struct spi_slave *slave;

	slave = (struct spi_slave *)malloc(sizeof(*slave));
	if (NULL == slave)
		return NULL;

	slave->master = NULL;
	list_head_init(&slave->msg_qu);

	return slave;
}

int spi_slave_attach(struct spi_master *master, struct spi_slave *slave)
{
	list_add_tail(&slave->slave_node, &master->slave_list);
	slave->master = master;

	return 0;
}

int spi_master_register(struct spi_master *master)
{
	int i;

	for (i = 0; i < MAX_SPI; i++) {
		if (NULL == g_spi_masters[i]) {
			g_spi_masters[i] = master;
			master->bus_num = i;

			return 0;
		}
	}

	return -EBUSY;
}

static struct spi_master *g_spi_master;
static int g_spi_master_num;

static struct spi_slave  *g_spi_slave;
static int  g_spi_slave_num;

int set_master_tab(struct spi_master spi_master[], int len)
{
	g_spi_master_num = len;
	g_spi_master = spi_master;

	return 0;
}

int set_slave_tab(struct spi_slave spi_slave[], int len)
{
	g_spi_slave_num = len;
	g_spi_slave = spi_slave;

	return 0;
}

struct spi_master *get_spi_master(char *name)
{
	int i;

	for (i = 0; i < g_spi_master_num; i++) {
		if (!strcmp(g_spi_master[i].name, name))
			return g_spi_master + i;
	}

	return NULL;
}

struct spi_slave *get_spi_slave(char *name)
{
	int i;

	for (i = 0; i < g_spi_slave_num; i++) {
		if (!strcmp(g_spi_slave[i].name, name))
			return g_spi_slave + i;
	}

	return NULL;
}

static int __INIT__ spi_init(void)
{
	int i;

	printf("%s:spi subsys init\n", __func__);

	for (i = 0; i < MAX_SPI; i++)
		g_spi_masters[i] = NULL;

	return 0;
}

SUBSYS_INIT(spi_init);
