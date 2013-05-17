#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#define LEN 512

int main(int argc, char *argv[])
{
	int fd;
	const char *file;
	char buff[LEN + 1];
	int len;

	if (argc < 2) {
		return -EINVAL;
	}

	file = argv[1];

	fd = open(file, O_RDONLY);
	if (fd < 0) {
		printf("Fail to open \"%s\"\n", file);
		return fd;
	}

	while ((len = read(fd, buff, LEN)) > 0) {
		buff[len] = '\0';
		printf("%s", buff);
	}

	close(fd);

	return 0;
}
