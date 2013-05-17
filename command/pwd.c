#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	char path[PATH_MAX];

	if (getcwd(path, PATH_MAX) != NULL)
		printf("%s\n", path);

	return 0;
}
