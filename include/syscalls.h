#pragma once

long sys_mount(const char *, unsigned long, const char *, const char *);
long sys_getcwd(char *buf, unsigned long size);
long sys_chdir(const char *filename);
struct linux_dirent;
int sys_getdents(unsigned int, struct linux_dirent *, unsigned int);
long sys_mkdir(const char *name, unsigned int /*fixme*/ mode);
