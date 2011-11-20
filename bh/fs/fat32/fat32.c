#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <fs/fs.h>
#include "fat32.h"

static ssize_t fat_read_block(struct fat_fs *fs, void *buff, int blk_no, size_t off, size_t size)
{
	struct block_device *bdev = fs->bdev;
	struct disk_drive *drive = container_of(bdev, struct disk_drive, bdev);

	size_t buf_len = (off + size + drive->sect_size - 1) & ~(drive->sect_size - 1);
	char blk_buf[buf_len];
	int start_blk, cur_blk;

	start_blk = blk_no * fs->clus_size / drive->sect_size;

	for (cur_blk = 0; cur_blk < buf_len / drive->sect_size; cur_blk++)
	{
		drive->get_block(drive, (start_blk + cur_blk) * drive->sect_size, blk_buf + cur_blk * drive->sect_size);
	}

	memcpy(buff, blk_buf + off, size);

	return size;
}

static __u32 fat_get_fat_table(struct fat_fs *fs, __u32 fat_num)
{
	int ret;
	static __u32 old_fat = ~0;
	static __u32 fat_catch[2048];

	if (old_fat == ~0 || fat_num < old_fat || fat_num > (fs->clus_size / sizeof(fat_num) + old_fat - 1))
	{
		ret = fat_read_block(fs, fat_catch, fs->dbr.resv_size / fs->dbr.sec_per_clus + fat_num * sizeof(fat_num) / (fs->clus_size),
			0, fs->clus_size);

		if (ret < 0)
			return ret;

		old_fat = (fat_num / fs->clus_size / sizeof(fat_num)) * (fs->clus_size / sizeof(fat_num));
	}

	return fat_catch[fat_num % (fs->clus_size / sizeof(fat_num))];
}

static int fat_mount(struct file_system_type *fs_type, unsigned long flags, struct block_device *bdev)
{
	int ret;
	__u16 blk_size;
	__u32 clus_size;
	__u32 data_off;
	struct fat_fs *fs;
	struct fat_boot_sector *dbr;
	__u32 root;

	struct disk_drive *drive = container_of(bdev, struct disk_drive, bdev);

	fs = malloc(sizeof(*fs));

	if (fs == NULL)
	{
		DPRINT("%s(): no mem\n", __func__);
		return -ENOMEM;
	}

	dbr = &fs->dbr;

	ret = drive->get_block(drive, 0, dbr);
	if (ret < 0)
	{
		DPRINT("%s(): read dbr err\n", __func__);
		return ret;
	}

	blk_size = dbr->sector_size[1] << 8 | dbr->sector_size[0];

	if (blk_size < 512 || blk_size > 4096 || (blk_size & (blk_size - 1)))
	{
		DPRINT("%s %s() line %d\n", __FILE__, __func__, __LINE__);
		return -EINVAL;
	}

	// TODO: check if bk_size != drive->sect_size ...

	data_off = (dbr->resv_size + dbr->fats * dbr->fat32_length) / dbr->sec_per_clus - 2;

	clus_size = blk_size * dbr->sec_per_clus; // fat block size

	root = dbr->root_cluster;

	fs->root = root;
	fs->bdev = bdev;
	fs->data = data_off;
	fs->clus_size = clus_size;

	bdev->fs = fs;
	drive->sect_size = blk_size; // fixme

	return 0;
}

#if 0
static int fat_umount(struct fat_fs *fs, const char *path, const char *type, unsigned long flags)
{
	// free
	return 0;
}
#endif

static int strxcpy(char *dst, const char *src, __u32 n)
{
	int i, j;

	for (i = 0, j = 0; i < n; i += 2)
	{
		if (*(src + i) != '\0')
		{
			*(dst + j) = *(src + i);
			j++;
		}
		else if (*(src + i) == '\0' && *(src + i + 1) == '\0')
		{
			*(dst + j) = '\0';
			j++;
		}
	}

	return j;
}

static int fat_find_next_file(struct fat_fs *fs,
			__u32 *block_num, char *name, struct fat_dentry *dir)
{
	char buf[fs->clus_size];
	static struct fat_dentry *dir_pos;
	static __u32 old_block_num;

	name[0] = '\0';

	do
	{
		if (*block_num != old_block_num
			|| dir_pos - (struct fat_dentry *)buf == fs->clus_size / sizeof(*dir_pos)) // find next cluster
		{
			old_block_num = *block_num;

			if (*block_num > 0x0ffffff8 || *block_num == 0)
			{
				return -1;
			}

			if (dir_pos - (struct fat_dentry *)buf == fs->clus_size / sizeof(*dir_pos))
			{
				*block_num = fat_get_fat_table(fs, *block_num);
				if (*block_num < 0)
				{
					goto L1;
				}
				old_block_num = *block_num;
			}

			fat_read_block(fs, buf, fs->data + *block_num, 0, fs->clus_size);

			dir_pos = (struct fat_dentry *)buf;
		}

		DPRINT("name = %s, attribute = %d\n", dir_pos->name, dir_pos->attr);

		switch ((__u8)dir_pos->name[0])
		{
		case 0xe5:
			break;

		case 0x0:
			goto L1;

		default:
			switch (dir_pos->attr)
			{
			case 0x10:
			case 0x20:
				if (!name[0])
				{
					int i, j;

					for (i = 0; i < 8 && dir_pos->name[i] != ' '; i++)
							name[i] = dir_pos->name[i];

					name[i++] = '.'; // fixme

					for (j = 8; j < 11 && dir_pos->name[j] != ' '; j++, i++)
						name[i]	= dir_pos->name[j];

					*(__u16 *)(name + i) = 0;
				}

				memcpy(dir, dir_pos, sizeof(*dir_pos));
				dir_pos++;
				return 1;

			case 0x0f:
				strxcpy(name + ((dir_pos->name[0] & 0x0f) - 1) * 13, dir_pos->name + 1, 10);
				strxcpy(name + ((dir_pos->name[0] & 0x0f) - 1) * 13 + 5, dir_pos->name + 14, 12);
				strxcpy(name + ((dir_pos->name[0] & 0x0f) - 1) * 13 + 11, dir_pos->name + 28, 4);
				break;

			default:
				break;
			}
		}

		dir_pos++;
	}while(dir_pos->name[0]);

L1:
	return -1;
}

static struct fat_dentry *fat_lookup(struct fat_fs *fs, __u32 parent, const char *name)
{
	struct fat_dentry *dir;
	int ret;
	char sname[MAX_FILE_NAME_LEN];

	dir = (struct fat_dentry *)malloc(sizeof(*dir));
	if (dir == NULL)
	{
		DPRINT("%s(): malloc err\n", __func__);
		return NULL;
	}

	while (1)
	{
		// fixme
		ret = fat_find_next_file(fs, &parent, sname, dir);
		printf("%s(): ret = %d, sname = %s\n",
			__func__, ret, sname);
		if (ret < 0)
		{
			DPRINT("can't find file\n");
			goto err;
		}

		if (strcmp(sname, name) == 0)
		{
			name = strchr(name, '/');
			if (name != NULL)	// last file
			{
				if (dir->attr == 0x10)
				{
					parent = dir->clus_hi << 16 | dir->clus_lo;
					continue;
				}
				else
				{
					goto err;
				}
			}
			return dir;
		}
	}

err:
	free(dir);
	return NULL;
}

#define MAX_FILE_NAME_SIZE 256

// fixme
static struct file *fat_open(void *file_sys, const char *name, int flags, int mode)
{
	struct fat_dentry *dir;
	struct fat_file *fp;
	struct fat_fs *fs = file_sys;

	dir = fat_lookup(fs, fs->root, name);

	if (!dir)
		return NULL;

	fp = malloc(sizeof(*fp));
	// if NULL

	fp->fs = fs;
	fp->dent = dir;

	return &fp->f;
}

static int fat_close(struct file *fp)
{
	struct fat_file *fat_fp = container_of(fp, struct fat_file, f);

	free(fat_fp);

	return 0;
}

static int fat_read(struct file *fp, void *buff, size_t size)
{
	__u32 clus_num, clus_size;
	size_t pos;
	size_t count = 0;
	size_t tmp_size;
	struct fat_fs *fs;
	struct fat_file *fat_fp = container_of(fp, struct fat_file, f);

	fs = fat_fp->fs;

	clus_size = fat_fp->fs->clus_size;
	clus_num = fat_fp->dent->clus_hi << 16 | fat_fp->dent->clus_lo;
	pos = fp->pos;

	if (size + pos > fat_fp->dent->size)
	{
		size = fat_fp->dent->size - pos;
	}

	while (pos >= clus_size)
	{
		clus_num = fat_get_fat_table(fs, clus_num);

#if 0
		if (clus_num < 0)
			return clus_num;
#endif

		pos -= clus_size;
	}

	while (size > count)
	{
		if (size + pos - count > clus_size)
		{
			tmp_size = clus_size - pos;
		}
		else
		{
			tmp_size = size - count;
		}

		tmp_size = fat_read_block(fs, buff + count, fs->data + clus_num, pos, tmp_size);
		if (tmp_size < 0)
		{
			break;
		}
		count += tmp_size;

		clus_num = fat_get_fat_table(fs, clus_num);
		if (clus_num < 0)
		{
			break;
		}

		pos = 0;
	}

	return count;
}

static int fat_write(struct file *fp, const void *buff, size_t size)
{
	return 0;
}

static const struct file_operations fat_fops =
{
	.open  = fat_open,
	.close = fat_close,
	.read  = fat_read,
	.write = fat_write,
};

static struct file_system_type fat_fs_type =
{
	.name  = "vfat",
	.mount = fat_mount,
	.fops  = &fat_fops,
};

#ifdef __G_BIOS__
static int __INIT__ fat32_init(void)
#else
int fat32_init(void)
#endif
{
	return file_system_type_register(&fat_fs_type);
}

#ifdef __G_BIOS__
SUBSYS_INIT(fat32_init);
#endif
