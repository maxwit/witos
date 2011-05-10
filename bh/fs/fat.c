#include <malloc.h>
#include <errno.h>
#include <string.h>
#include "fat.h"

int fat_mount(struct block_device *bdev, const char *type, unsigned long flags)
{
	int ret;
	__u16 blk_size;
	__u32 clus_size;
	__u32 data_off;
	struct fat_fs *fs;
	struct fat_boot_sector *dbr;
	struct fat_dentry *root;
	__u32  *fat;
	size_t fat_len;
	struct generic_drive *drive = container_of(bdev, struct generic_drive, blk_dev);

	fs = malloc(sizeof(*fs));

	dbr = &fs->dbr; // mark
	// if ...

	ret = drive->get_block(drive, 0, dbr);
	// if ...

	blk_size = dbr->sector_size[1] << 8 | dbr->sector_size[0];

	if (blk_size < 512 || blk_size > 4096 || (blk_size & (blk_size - 1)))
	{
		DPRINT("%s %s() line %d\n", __FILE__, __func__, __LINE__);
		return -EINVAL;
	}

	// TODO: check if bk_size != drive->sect_size ...

	data_off = (dbr->resv_size + dbr->fats * dbr->fat32_length) * blk_size;

	clus_size = blk_size * dbr->sec_per_clus;

	root = malloc(clus_size);
	// if

	// part_read(part, root, data_off, clus_size);
	// while
	ret = drive->get_block(drive, data_off, root);

	//
	fat_len = sizeof(*fat) * dbr->fat32_length * blk_size;
	fat = malloc(fat_len);
	if (!fat)
	{
		DPRINT("%s(): no mem\n", __func__);
		return -ENOMEM;
	}

	// part_read(part, fat, dbr->resv_size * blk_size, fat_len);
	// while ...
	ret = drive->get_block(drive, dbr->resv_size * blk_size, fat);

	fs->fat  = fat;
	fs->root = root;
	fs->bdev = bdev;
	fs->data = data_off;

	bdev->fs = fs;

	return 0;
}

int fat_umount(struct part_attr *part, const char *path, const char *type, unsigned long flags)
{
	// free
	return 0;
}

#define FAT_FNAME_LEN 13

static struct fat_dentry *fat_lookup(struct fat_dentry *parent, const char *name)
{
	struct fat_dentry *dir = parent;
	char sname[FAT_FNAME_LEN];

	while (dir->name[0])
	{
		int i = 0, j = 0;

		if (dir->name[0] == (char)0xE5)
		{
			dir++;
			continue;
		}

		while (j < 11 && dir->name[j] != '\0')
		{
			if (dir->name[j] == ' ')
			{
				if (j >= 8)
					break;

				j = 8;
			}
			else
			{
				if (j == 8)
					sname[i++] = '.';

				sname[i++] = dir->name[j++];
			}
		}

		sname[i] = '\0';

		if (!strcasecmp(sname, name))
		{
			return dir;
		}
		// printf("%s(): file = %s\n", __func__, sname);

		dir++;
	}

	return NULL;
}

// name = "mmcblock0p1:a.c"
struct file *fat_open(const char *name, int flags, ...)
{
	struct fat_dentry *dir;
	struct file *fp;
	struct block_device *bdev;

	//
	dir = fat_lookup(bdev->fs->root, name);
	if (!dir)
		return NULL;

	fp = malloc(sizeof(*fp));

	fp->dent = dir;
	fp->offset = 0;

	return fp;
}

int fat_close(struct file *fp)
{
	free(fp);
	return 0;
}

int fat_read(struct file *fp, void *buff, size_t size)
{
	__u32 clus_num, clus_size;
	struct fat_dentry *dir = fp->dent;
	struct fat_boot_sector *dbr;
	struct fat_fs *fs;

	// fs = ..
	dbr = &fs->dbr;

	clus_num = dir->clus_hi << 16 | dir->clus_lo;
	printf("%s(): cluster = %d\n", __func__, clus_num);

	//
	clus_size = fs->bdev.bdev_size * dbr->sec_per_clus;

	if (size > dir->size)
		size = dir->size;

	// part_read(part, buff,  fs->data + (clus_num - 2) * clus_size + fp->offset, size);

	fp->offset += size;

	return size;
}

int fat_write(struct file *fp, const void *buff, size_t size)
{
	return 0;
}
