#include <stdio.h>
#include <errno.h>
#include <fs/fs.h>

int main(int argc, char *argv[])
{
	int ret;

	if (argc != 2)
	{
		printf("Usage:\n\t%s device\n", argv[1]);
		return -EINVAL;
	}

	ret = mount("vfat", 0, argv[1], NULL);

	return ret;
}
