#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

void __WEAK__ reset(void);

int main(int argc, char *argv[])
{
	// TODO: add code here!

	if (reset)
		reset();
	else {
		printf("H/W reset not supported, trying to S/W reset ...\n");

		((void (*)(void))CONFIG_GTH_START_MEM)(); // fix for OMAP3
	}

	return -EFAULT;
}
