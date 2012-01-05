#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <block.h> //  fixme: to be removed!

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
	DIR *dir;

	dir = zalloc(sizeof(*dir));
	if (!dir)
		return NULL;

	return dir;
}

struct dirent *readdir(DIR *dir)
{
	struct dirent *de = NULL;

	assert(dir);
	if (dir->iter == dir->list)
		return NULL;

	dir->iter = dir->iter->next;

	return de;
}

int closedir(DIR *dir)
{
	assert(dir);

	free(dir);
	return 0;
}
