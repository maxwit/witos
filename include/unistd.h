#pragma once

#include <getopt.h>

#define PATH_MAX   256

int GAPI chdir(const char *path);

char * GAPI getcwd(char *buff, size_t size);

char * GAPI get_current_dir_name();

int GAPI mkdir(const char *name, unsigned int mode);
