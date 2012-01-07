#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <fs/fs.h>

static char g_cwd[PATH_MAX];

// fixme
int chdir(const char *path)
{
	DIR *dir;
	struct dirent *de;

	dir = opendir(DEV_ROOT);
	if (!dir)
		return -ENOENT;

	while ((de = readdir(dir))) {
		if (!strcmp(path, de->d_name)) {
			strncpy(g_cwd, path, PATH_MAX);
			closedir(dir);
			return 0;
		}
	}

	return -ENOENT;
}

char *getcwd(char *buff, size_t size)
{
	return strncpy(buff, g_cwd, min(size, PATH_MAX));
}

char *get_current_dir_name()
{
	return strdup(g_cwd);
}

const char *__getcwd(void)
{
	return g_cwd;
}

DIR *opendir(const char *name)
{
	int fd;
	DIR *dir;

	fd = open(name, O_RDONLY);
	if (fd < 0)
		return NULL;

	dir = zalloc(sizeof(*dir));
	if (!dir)
		return NULL;

	dir->fd = fd;

	return dir;
}

struct dirent *readdir(DIR *dir)
{
	int ret;
	struct dirent *de;
	struct linux_dirent lde;

	assert(dir);

	ret = getdents(dir->fd, &lde, 1); // fixme
	if (ret <= 0)
		return NULL;

	de = malloc(lde.d_reclen);
	if (!de)
		return NULL;

	de->d_ino    = lde.d_ino;
	de->d_off    = lde.d_off;
	de->d_reclen = lde.d_reclen;
	de->d_type   = lde.d_type; // fixme
	strcpy(de->d_name, lde.d_name);

	return de;
}

int closedir(DIR *dir)
{
	assert(dir);

	close(dir->fd);
	free(dir);

	return 0;
}
