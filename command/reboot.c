#include <errno.h>
#include <stdio.h>

void __WEAK__ reset(void);

int main(int argc, char *argv[])
{
	if (reset)
		reset();

	printf("H/W reset not supported, trying to S/W reset ...\n");

	return -EFAULT;
}
