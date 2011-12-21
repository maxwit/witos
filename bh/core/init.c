#include <shell.h>
#include <sysconf.h>
// fixme: to be removed!
#include <net/net.h>
#include <uart/uart.h>
#include <font/font.h>

static const char banner[] = "\n\n" // CLRSCREEN
	"\t+---------------------------------+\n"
	"\t|    Welcome to MaxWit g-bios!    |\n"
	"\t|      http://www.maxwit.com      |\n"
	"\t|      "__DATE__", "__TIME__"      |\n"
	"\t+---------------------------------+\n"
	"\ng-bios running "
#ifdef CONFIG_IRQ_SUPPORT
	"with IRQ enabled!\n"
#else
	"in polling mode!\n"
#endif
	;

static int __INIT__ system_init(void)
{
	int count;
	init_func_t *init_call;
	extern init_func_t init_call_begin[], init_call_end[];

	count = 1;
	init_call = init_call_begin;

	while (init_call < init_call_end) {
		int ret;
		const char *func_name;

		printf("%d. [0x%08x]", count, *init_call);
		func_name = get_func_name(*init_call);
		if (func_name)
			printf(" %s()", func_name);
		putchar('\n');

		ret = (*init_call)();
		if (ret < 0)
			puts("Failed!");
		else
			puts("OK!");

		init_call++;
		count++;

		printf("\n");
	}

	printf("(g-bios initialization finished.)\n");

	// show system information
	printf("%s\n", banner);

	return 0;
}

// fixme: by sysconfig
#ifdef CONFIG_AUTO_BOOT
static void __INIT__ auto_boot(void)
{
	int time_out = 3;
	char *argv[] = {"boot"};

	while (1) {
		int index;

		printf("\rAutoboot after %d seconds. "
			"Press any key to interrupt.", time_out);

		if (0 == time_out)
			break;

		for (index = 0; index < 10; index++) {
			mdelay(1000);

			if (uart_rxbuf_count()) {
				puts("\n");
				return;
			}
		}

		time_out--;
	}

	puts("\n");

	exec(ARRAY_ELEM_NUM(argv), argv);
}
#endif

int main(void)
{
	int ret;

	ret = conf_load();
	if (ret < 0) {
		printf("Warning: fail to initialize system configuration!\n"
			"Trying reset to default!\n");

		conf_reset();
	}

	system_init();

	conf_store();

#ifdef CONFIG_AUTO_BOOT
	auto_boot();
#endif

	shell();

	return -EINVAL;
}
