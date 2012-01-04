#pragma once

#include <types.h>

int demo_open(const char *name, int flags, ...);

int demo_close(int fd);

ssize_t demo_read(int fd, void *buff, size_t count);

ssize_t demo_write(int fd, const void *buff, size_t count);

int demo_ioctl(int fd, int cmd, ...);

loff_t demo_lseek(int fd, loff_t offset, int whence);

// fixme
int demo_mount(const char *type, unsigned long flags, const char *bdev_name, const char *path);

int demo_umount(const char *mnt);
