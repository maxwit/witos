#include <io.h>
#include <stdio.h>
#include <delay.h>
#include <errno.h>

#define I2C_TIMEOUT 10

#define omap_i2c_outb(v, a) \
	writeb(VA(I2C_BASE1 + a), v)

#define omap_i2c_outw(v, a) \
	writew(VA(I2C_BASE1 + a), v)

#define omap_i2c_inb(a) \
	readb(VA(I2C_BASE1 + a))

#define omap_i2c_inw(a) \
	readw(VA(I2C_BASE1 + a))

static __u16 i2c_wait_ready (void)
{
	__u16 stat;
	int to;

	for (to = 0; to < I2C_TIMEOUT; to++) {
		udelay (1000);
		stat = omap_i2c_inw(I2C_STAT);
		if (stat & 0xC1F)
			break;
	}

	if (I2C_TIMEOUT == to) {
		printf ("timed out in i2c_wait_ready: I2C_STAT=%x\n",
			omap_i2c_inw(I2C_STAT));
			omap_i2c_outw(0xFFFF, I2C_STAT);
	}

	return stat;
}

static inline void i2c_flush(void)
{
	__u16 stat;

	while(1){
		stat = omap_i2c_inw(I2C_STAT);
		if (stat != I2C_STAT_RRDY)
			break;

		omap_i2c_inb(I2C_DATA);
		omap_i2c_outw(I2C_STAT_RRDY, I2C_STAT);
		udelay(1000);
	}
}

int i2c_write_byte(__u8 addr, __u8 reg, __u8 val)
{
	int errno = 0, to;
	__u16 stat;

	omap_i2c_outw(0xFFFF, I2C_STAT);

	for (to = 0; to < I2C_TIMEOUT; to++) {
		stat = omap_i2c_inw(I2C_STAT);
		if (!(stat & I2C_STAT_BB))
			break;

		omap_i2c_outw(stat, I2C_STAT);
		udelay (50000);
	}

	if (I2C_TIMEOUT == to) {
		GEN_DBG("I2C bus busy! stat = 0x%08x\n", stat);
		return -EBUSY;
	}

	omap_i2c_outw(0xFFFF, I2C_STAT);

	omap_i2c_outw(2, I2C_CNT);
	omap_i2c_outw(addr, I2C_SA);
	omap_i2c_outw(0x8603, I2C_CON);

	stat = i2c_wait_ready();
	if (!(stat & I2C_STAT_XRDY))
		return -ETIMEDOUT;

	omap_i2c_outb(reg, I2C_DATA);
	omap_i2c_outw(I2C_STAT_XRDY, I2C_STAT);

	stat = i2c_wait_ready();
	if (!(stat & I2C_STAT_XRDY))
		return -ETIMEDOUT;

	omap_i2c_outb(val, I2C_DATA);
	omap_i2c_outw(I2C_STAT_XRDY, I2C_STAT);

	udelay (50000);
	stat = omap_i2c_inw(I2C_STAT);
	if (stat & I2C_STAT_NACK)
		return -EIO;

	to = 200;
	omap_i2c_outw(I2C_CON_EN, I2C_CON);
	while ((stat = omap_i2c_inw(I2C_STAT)) || \
		(omap_i2c_inw(I2C_CON) & I2C_CON_MST)) {
		udelay (1000);
		omap_i2c_outw(0xFFFF, I2C_STAT);

		to--;
		if(to == 0)
			break;
	}

	i2c_flush();
	omap_i2c_outw(0xFFFF, I2C_STAT);
	omap_i2c_outw(0, I2C_CNT);

	return errno;
}

void i2c_disp_init(void)
{
	__u16 val;

	omap_i2c_outw(0x2, I2C_SYSC);
	udelay(1000);
	omap_i2c_outw(0x0, I2C_SYSC);

	val = omap_i2c_inw(I2C_CON);
	if (val & I2C_CON_EN) {
		omap_i2c_outw(0, I2C_CON);
		udelay (50000);
	}

	omap_i2c_outw(0, I2C_PSC);

	val = 0xfff9;
	omap_i2c_outw(val, I2C_SCLL);
	omap_i2c_outw(val, I2C_SCLH);

	omap_i2c_outw(1, I2C_OA);
	omap_i2c_outw(I2C_CON_EN, I2C_CON);

	omap_i2c_outw(0x1F, I2C_IE);

	i2c_flush();

	omap_i2c_outw(0xFFFF, I2C_STAT);
	omap_i2c_outw(0, I2C_CNT);

	// init disp
	i2c_write_byte (0x49, 0x9B, 0x80);
	i2c_write_byte (0x49, 0x9E, 0x80);
}
