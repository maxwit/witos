#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <block.h>

int main(int argc, char *argv[])
{
	int opt;
	const char *path;
	DIR *dir;
	struct dirent *entry;

	while ((opt = getopt(argc, argv, "l")) != -1) {
		switch (opt) {
		case 'l':
			break;

		default:
			usage();
			return -EINVAL;
		}
	}

	if (optind + 1 < argc) {
		usage();
		return -EINVAL;
	}

	if (optind < argc) {
		path = argv[optind];
	} else {
		path = ".";
	}
	dir = opendir(path);
	if (!dir) {
		printf("cannot access \"%s\"\n", path);
		return -ENOENT;
	}

	while ((entry = readdir(dir))) {
		printf("%s\n", entry->d_name);
	}

	closedir(dir);

	return 0;
}
