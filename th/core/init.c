/*
 *  comment here
 */

#include <stdio.h>
#include <delay.h>
#include <loader.h>
#include <uart/uart.h>

extern struct loader_opt g_loader_begin[], g_loader_end[];

static int load_gbios_bh(char key);

int main(void)
{
	int i;

	read_cpu_id();

#ifdef CONFIG_SDRAM_TESTING
	volatile __u32 *p = (__u32 *)SDRAM_BASE;

	while (p < (__u32 *)(SDRAM_BASE + SDRAM_SIZE)) {
		__u32 val1 = *p, val2;

		val2 = ~val1;
		*p = val2;
		val1 = *p;

		if (val1 != val2)
			printf("bad memory @ 0x%x\n", p);

		p++;
	}
#endif

	for (i = 0; i < 0x100; i++) {
		char key;

		if (uart_rxbuf_count() > 0) {
			key = uart_recv_byte();

#ifdef CONFIG_LOADER_MENU
			do {
				char nkey;
				struct loader_opt *loader;
#endif

				load_gbios_bh(key);

#ifdef CONFIG_LOADER_MENU
				printf("\nGTH Menu:\n");

				for (loader = g_loader_begin; loader < g_loader_end; loader++)
					printf("  [%c] %s\n", loader->ckey[0], loader->prompt);
				printf("  [q] Default\nEnter Your Choice\n");

				while (key == (nkey = uart_recv_byte()));

				key = nkey;
			} while ('q' != key);

			break;
#endif
		}

		udelay(0x1000);
	}

	load_gbios_bh(CONFIG_DEFAULT_LOADER);

	return 'm';
}

static inline void boot_bh()
{
	printf("\n");
	((void (*)())CONFIG_GBH_START_MEM)();
}

int load_gbios_bh(char key)
{
	int ret;
	struct loader_opt *loader;

	if ('A' <= key && key <= 'Z')
		key += 'a' - 'A';

	for (loader = g_loader_begin; loader < g_loader_end; loader++) {
#ifdef CONFIG_DEBUG
		printf("%c: 0x%x\n", loader->ckey[0], loader->main);
#endif

		if (loader->ckey[0] == key) {
			// fixme: prompt for RAM/ROM
			printf("Loading from %s ...\n", loader->prompt);

			loader->load_addr = (void *)CONFIG_GBH_START_MEM;

			ret = loader->main(loader);
			if (ret < 0)
				return ret;

			boot_bh();
		}
	}

	return 0;
}

#ifdef CONFIG_RAM_LOADER
static int ram_loader(struct loader_opt *loader)
{
	boot_bh();
	return -1;
}

REGISTER_LOADER(m, ram_loader, "RAM/ROM");
#endif
