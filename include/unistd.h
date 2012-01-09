#pragma once

#include <getopt.h>

#define PATH_MAX   256

int chdir(const char *path);

char *getcwd(char *buff, size_t size);

char *get_current_dir_name();
