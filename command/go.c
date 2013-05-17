#include <go.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
	void (*go)(void);

	if (argc == 1) {
		go = go_get_addr();
	} else if (str_to_val(argv[1], (unsigned long *)&go) < 0) {
		usage();
		return -EINVAL;
	}

	printf("goto 0x%p ...\n", go);

	go();

	return -EIO;
}
