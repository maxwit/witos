#pragma once

long sys_mount(const char *, unsigned long, const char *, const char *);
long sys_getcwd(char *buf, unsigned long size);
long sys_chdir(const char *filename);
long sys_mkdir(const char *name, unsigned int /*fixme*/ mode);
