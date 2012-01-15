#pragma once

#include <list.h>

#define MAX_DENT_NAME_SIZE  256

// g-bios system standard directory
#define DEV_ROOT   "/dev"

typedef unsigned long ino_t;

typedef struct {
	int fd;
} DIR;

// copy from Linux man page
struct dirent {
	ino_t          d_ino;       /* inode number */
	loff_t         d_off;       /* offset to the next dirent */
	unsigned short d_reclen;    /* length of this record */
	unsigned char  d_type;      /* type of file; not supported by all file system types */
	char           d_name[256]; /* filename */
};

DIR *GAPI opendir(const char *name);
struct dirent * GAPI readdir(DIR *dir);
int GAPI closedir(DIR *dir);
