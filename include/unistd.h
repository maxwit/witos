#pragma once

#include <getopt.h>

#define PATH_MAX   256

int chdir(const char *path);

char *getcwd(/* fixme */);
