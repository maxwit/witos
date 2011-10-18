#include <string.h>
#include <getopt.h>
#include <block.h>

static void usage()
{
	printf("Usage: ...\n");
}

void set_pwd(const char * cwd);

const char *get_pwd();

int main(int argc, char *argv[])
{
	struct block_device *bdev;
	const char *name;
	printf("%s\n", get_pwd());

	if (argc < 1)
	{
		usage();
		return -EINVAL;
	}
	name = argv[1];

	bdev = get_bdev_by_name(name);
	if (!bdev)
	{
		printf("no such block device (\"%s\")! pwd = %s\n", name, get_pwd());
		return -ENODEV;
	}

	set_pwd(name);

	return 0;
}
