#include <net/net.h>
#include <net/mii.h>
#include <irq.h>

#include "lan9220.h"

static __INIT__ int lan9220_init(void)
{
	u32 word;

	printf("%s(), line : %d\n", __func__, __LINE__);
	word = readl(VA(LAN9220_BASE + ID_REV));
	printf("lan9220 chip id : %x\n", word);

	return 0;
}

DRIVER_INIT(lan9220_init);
