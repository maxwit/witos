#pragma once

#include <types.h>

int __open(const char *name, int flags, ...);

int __close(int fd);

ssize_t __read(int fd, void *buff, size_t count);

ssize_t __write(int fd, const void *buff, size_t count);

int __ioctl(int fd, int cmd, ...);

loff_t __lseek(int fd, loff_t offset, int whence);

// fixme
int __mount(const char *type, unsigned long flags, const char *bdev_name, const char *path);

int __umount(const char *mnt);
