#pragma once


struct fat_boot_sector {
	__u8	ignored[3];
	__u8	system_id[8];
	__u8	sector_size[2];
	__u8	sec_per_clus;
	__u16	resv_size;
	__u8	fats;
	__u8	dir_entries[2];
	__u8	sectors[2];
	__u8	media;
	__u16	fat_length;
	__u16	secs_track;
	__u16	heads;
	__u32	hidden;
	__u32	total_sect;
	__u32	fat32_length;
	__u16	flags;
	__u8	version[2];
	__u32	root_cluster;
	__u16	info_sector;
	__u16	backup_boot;
	__u8	resv2[12];
	__u8    drv_num;
	__u8    resv3;
	__u8    boot_sign;
	__u8    vol_id[4];
	__u8    vol_lab[11];
	__u8    fs_type[8];
	__u8    resv4[418];
	__u8    blk_sign[2];
};

struct fat_dentry
{
	char name[11];
	__u8 attr;
	__u8 resv1;
	__u8 ctime;
	__u8 resv2[6];
	__u16 clus_hi;
	__u16 wtime;
	__u16 wdate;
	__u16 clus_lo;
	__u32 size;
};

struct fat_fs
{
	struct fat_boot_sector *dbr;
	struct fat_dentry *root;
	struct part_attr *part;
	__u32  *fat;
	__u32  data;
	__u32  blk_size;
};

struct file
{
	struct fat_dentry *dent;
	size_t offset;
	int    mode;
};

int fat_mount(struct part_attr *part, const char *path, const char *type, unsigned long flags);

#define O_RDONLY 0
#define O_RDWD   0

struct file *fat_open(const char *name, int flags, ...);

int fat_close(struct file *fp);

int fat_read(struct file *fp, void *buff, size_t size);

int fat_write(struct file *fp, const void *buff, size_t size);
