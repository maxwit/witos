#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>

#if 0
static int get_bdev_by_index(int index, char name[], size_t size)
{
	DIR *dir;
	struct dirent *de;

	dir = opendir(DEV_ROOT);
	if (!dir)
		return -ENOENT;

	while ((de = readdir(dir))) {
		int i = 1, num = 0;
		const char *postfix;

		postfix = de->d_name + strlen(de->d_name) - 1;

		while (postfix >= de->d_name) {
			if (!ISDIGIT(*postfix))
				break;

			num += i * (*postfix - '0');
			i *= 10;
			postfix--;
		}

		if (num == index) {
			strncpy(name, de->d_name, size);
			return 0;
		}
	}

	closedir(dir);
	return -ENODEV;
}
#endif

int main(int argc, char *argv[])
{
	int ret; // index;
	const char *path;
	char str[CONF_VAL_LEN];

	switch (argc) {
	case 1:
		ret = conf_get_attr(HOME, str);
		assert(ret >= 0); // should never fail!
		path = str;
		goto L1;

	case 2:
		path = argv[1];
		break;

	default:
		usage();
		return -EINVAL;
	}

#if 0
	ret = dec_str_to_int(path, &index);
	if (ret >= 0) {
		if (index >= 0) {
			ret = get_bdev_by_index(index, str, sizeof(str));
			if (!ret) {
				path = str;
				goto L1;
			}
		}

		usage();
		return -ENOENT;
	}
#endif

L1:
	if (!strcmp(path, get_current_dir_name()))
		return 0;

	ret = chdir(path);
	if (ret < 0) {
		printf("cd failed, errno = %d!\n", ret);
		return ret;
	}

	return ret;
}
