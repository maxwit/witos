#pragma once

#include <types.h>
#include <list.h>
#include <block.h>

struct vfsmount;
struct super_block;
struct inode;
struct dentry;
struct file;

struct qstr {
	const char *name;
	unsigned int len;
};

struct path {
	struct vfsmount *mnt;
	struct dentry *dentry;
};

struct nameidata {
	struct path path;
	// unsigned int flags;
	int ret;
};

struct file_system_type {
	const char *name;
	struct file_system_type *next;

	struct dentry *(*mount)(struct file_system_type *, unsigned long,
						const char *);
	void (*umount)(struct super_block *);
};

int file_system_type_register(struct file_system_type *);

struct file_system_type *file_system_type_get(const char *);

struct vfsmount {
	const char *dev_name;
	struct dentry *root;
	struct vfsmount *mnt_parent;
	struct dentry *mountpoint;
	struct file_system_type *fstype;
	struct list_node mnt_hash;
};

struct block_buff {
	// __u32  blk_id;
	__u8    *blk_base;
	size_t  blk_size;
	size_t  max_size;
	__u8    *blk_off;
};

struct linux_dirent;

struct file_operations {
	int (*open)(struct file *, struct inode *);
	int (*close)(struct file *);
	ssize_t (*read)(struct file *, void *, size_t, loff_t *);
	ssize_t (*write)(struct file *, const void *, size_t, loff_t *);
	int (*ioctl)(struct file *, int, unsigned long);
	loff_t (*lseek)(struct file *, loff_t, int);
	int (*readdir)(struct file *, struct linux_dirent *);
};

struct file {
	loff_t f_pos;
	unsigned int flags;
	// unsigned int mode;
	const struct file_operations *f_op;
	struct dentry *f_dentry;
	// struct vfsmount *vfsmnt;
	struct block_buff blk_buf; // for block device, to be removed!
	void *private_data;
};

#define S_IFMT  00170000
#define S_IFSOCK 0140000
#define S_IFLNK	 0120000
#define S_IFREG  0100000
#define S_IFBLK  0060000
#define S_IFDIR  0040000
#define S_IFCHR  0020000
#define S_IFIFO  0010000
#define S_ISUID  0004000
#define S_ISGID  0002000
#define S_ISVTX  0001000

#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)

#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

struct inode_operations {
	struct dentry *(*lookup)(struct inode *, struct dentry *, struct nameidata *);
	int (*create) (struct inode *, struct dentry *, int, struct nameidata *);
	int (*mkdir) (struct inode *, struct dentry *, int);
	int (*rmdir) (struct inode *, struct dentry *);
	int (*mknod)(struct inode *, struct dentry *, int);
};

struct inode {
	unsigned long i_ino;
	loff_t        i_size;
	__u32         i_mode;
	struct super_block            *i_sb;
	const struct inode_operations *i_op;
	const struct file_operations  *i_fop;
};

#define DNAME_INLINE_LEN 36

struct dentry {
	struct qstr d_name;
	char d_iname[DNAME_INLINE_LEN];
	struct inode *d_inode;
	struct super_block *d_sb;
	struct dentry *d_parent;
	struct list_node d_subdirs, d_child;
	// int d_type; // fixme
};

struct dentry *__d_alloc(struct super_block *sb, const struct qstr *str);

struct dentry *d_alloc(struct dentry *parent, const struct qstr *str);

struct dentry * d_alloc_root(struct inode *root_inode);

void dput(struct dentry *dentry);

// copy from Linux man page
struct linux_dirent {
	unsigned long  d_ino;	  /* Inode number */
	unsigned long  d_off;	  /* Offset to next linux_dirent */
	unsigned short d_reclen;  /* Length of this linux_dirent */
	char		   d_name[0];  /* Filename (null-terminated) */
	/* length is actually (d_reclen - 2 - offsetof(struct linux_dirent, d_name) */
	unsigned char  d_type; // fixme
};

int filldir(struct linux_dirent *, const char * name, int namlen, loff_t offset,
		   unsigned long ino, unsigned int type);

struct super_block {
	__u32 s_blksize;
	struct block_device *s_bdev;
	struct dentry *s_root;
	void *s_fs_info;
};

struct super_block *sget(struct file_system_type *type, void *data);

// fixme
int get_unused_fd();
int fd_install(int fd, struct file *fp);
struct file *fget(unsigned int fd);

//
struct fs_struct {
	struct path root, pwd;
};

void set_fs_root(const struct path *);
void set_fs_pwd(const struct path *);
void get_fs_root(struct path *);
void get_fs_pwd(struct path *);

int path_walk(const char *path, struct nameidata *nd);
int follow_up(struct path *path);

int vfs_mkdir(struct inode *dir, struct dentry *dentry, int mode);
int vfs_mknod(struct inode *dir, struct dentry *dentry, int mode);
