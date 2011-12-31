#pragma once

#define O_RDONLY   1
#define O_WRONLY   2
#define O_RDWR     3

int open(const char *name, int flags, ...);

int close(int fd);

ssize_t read(int fd, void *buff, size_t count);

ssize_t write(int fd, const void *buff, size_t count);

int ioctl(int fd, int cmd, ...);

#define SEEK_SET   1
#define SEEK_END   2
#define SEEK_CUR   3

loff_t lseek(int fd, loff_t offset, int whence);

// fixme
int mount(const char *type, unsigned long flags, const char *bdev_name, const char *path);

int umount(const char *mnt);
