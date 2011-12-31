#include <unistd.h>

static char *g_cwd; // fixme: with const

int chdir(const char *path)
{
	g_cwd = (char *)path;

	return 0;
}

char *getcwd(/* fixme */)
{
	return g_cwd;
}
