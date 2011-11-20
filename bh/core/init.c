#include <shell.h>
#include <sysconf.h>
// fixme: to be removed!
#include <net/net.h>
#include <uart/uart.h>

static const char banner[] = "\n\n" // CLRSCREEN
	"\t+---------------------------------+\n"
	"\t|    Welcome to MaxWit g-bios!    |\n"
	"\t|      http://www.maxwit.com      |\n"
	"\t|      "__DATE__", "__TIME__"      |\n"
	"\t+---------------------------------+\n"
#ifdef CONFIG_DEBUG
	"\ng-bios running "
#ifdef CONFIG_IRQ_SUPPORT
	"with IRQ enabled!\n"
#else
	"in polling mode!\n"
#endif
#endif
	;

static int __INIT__ sys_init(void)
{
	int count = 1;
	init_func_t *init_call = init_call_begin;
	const char* func_name;

	while (init_call < init_call_end) {
		int ret;

		printf("%d. [0x%08x]", count, *init_call);
		func_name = get_func_name(*init_call);
		if(func_name)
			printf(" %s()", func_name);
		putchar('\n');

		ret = (*init_call)();
#ifdef CONFIG_DEBUG
		if (ret < 0)
			puts("Failed!");
		else
			puts("OK!");
#endif

		init_call++;
		count++;

		printf("\n");
	}

	printf("(g-bios initialization finished.)\n");

	// show system information
	printf("%s\n", banner);

	ndev_check_link_status();
	printf("\n");

	return 0;
}

static void __INIT__ auto_boot(void)
{
	int time_out = 3;
	char *argv[] = {"boot"};

	while (1) {
		int index;

		printf("\rAutoboot after %d seconds. Press any key to interrupt.", time_out);
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

int main(void)
{
	int ret;

	ret = sysconf_init();
	if (ret < 0) {
		printf("fail to initialize system configuration!\n");
		return ret;
	}

	sys_init();

	// auto_boot();

	shell();

	return -EINVAL;
}
