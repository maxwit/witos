#include <stdio.h>
#include <errno.h>
#include <syscalls.h>

int main(int argc, char *argv[])
{
	if (argc < 2)
		return -EINVAL;

	sys_mkdir(argv[1], 0755);

	return 0;
}
