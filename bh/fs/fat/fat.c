#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <fs/fs.h>

#if 0
static void show_by_hex(void *buf, u32 size)
{
	__u8 *temp = (__u8 *)buf;
	int i;
	for (i = 0; i < size; i ++)
	{
		if (i % 16 == 0) printf("\n");
		printf("%02x ", temp[i]);
	}

	printf("\n");
	return ;
}
#endif

static ssize_t fat_read_block(struct fat_fs *fs, void *buff, int blk_no, size_t off, size_t size)
{
	struct block_device *bdev = fs->bdev;
	struct disk_drive *drive = container_of(bdev, struct disk_drive, bdev);

	size_t buf_len = (off + size + bdev->sect_size - 1) & ~(bdev->sect_size - 1);
	char blk_buf[buf_len];
	int start_blk, cur_blk;

	start_blk = blk_no * fs->clus_size / bdev->sect_size;

	for (cur_blk = 0; cur_blk < buf_len / bdev->sect_size; cur_blk++)
	{
		drive->get_block(drive, (start_blk + cur_blk) * bdev->sect_size, blk_buf + cur_blk * bdev->sect_size);
	}

	memcpy(buff, blk_buf + off, size);

	return size;
}

static __u32 fat_get_fat_table(struct fat_fs *fs, __u32 fat_num)
{
	int ret;
	static __u32 old_fat = ~0;
	static __u32 fat_catch[2048];

	if (old_fat == ~0 || fat_num <old_fat || fat_num > (fs->clus_size / sizeof(fat_num) + old_fat - 1))
	{
		ret = fat_read_block(fs, fat_catch, fs->dbr.resv_size/fs->dbr.sec_per_clus + fat_num * sizeof(fat_num)/ (fs->clus_size),
			0, fs->clus_size);
		if (ret < 0)
		{
			return -1;
		}

		old_fat = (fat_num / fs->clus_size / sizeof(fat_num)) * (fs->clus_size / sizeof(fat_num));
	}

	return fat_catch[fat_num % (fs->clus_size / sizeof(fat_num))];
}

int fat_mount(const char *type, unsigned long flags, struct block_device *bdev)
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

	data_off = (dbr->resv_size + dbr->fats * dbr->fat32_length) / dbr->sec_per_clus - 2;	//data 	//is easy to calc if begin 0

	clus_size = blk_size * dbr->sec_per_clus;	// fat block size

	root = dbr->root_cluster;

	fs->root = root;
	fs->bdev = bdev;
	fs->data = data_off;
	fs->clus_size = clus_size;

	bdev->fs = fs;
	bdev->sect_size = blk_size;

	return 0;
}

int fat_umount(struct fat_fs *fs, const char *path, const char *type, unsigned long flags)
{
	// free
	return 0;
}

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

static int fat_find_next_file(struct fat_fs *fs, __u32 *block_num, char *name, struct fat_dentry *dir, char *buf)
{
	char *fn_pos = name;
	static struct fat_dentry *dir_pos;
	static __u32 old_block_num;

	memset(fn_pos, 0, MAX_FILE_NAME_LEN);

	do
	{
		if ((*block_num != old_block_num) || dir_pos - (struct fat_dentry *)buf == fs->clus_size / sizeof(*dir_pos))	//find next cluster
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

		switch (*(char *)dir_pos)
		{
		case 0xe5:
			break;

		case  0:
			goto L1;
			break;

		default:
			switch (dir_pos->attr)
			{
			case 0x10:
			case 0x20:
				if (!fn_pos[0])
				{
					int temp1, temp2;

					temp1 = strchr((char *)dir_pos, ' ') - (char *)dir_pos;
					memcpy(fn_pos, (char *)dir_pos, temp1);
					temp2 = strchr((char *)dir_pos + 8, ' ') - (char *)dir_pos - 8;
					if (temp2 != 0)
					{
						*(fn_pos + temp1) = '.';
						temp1++;
						memcpy(fn_pos + temp1, (char *)dir_pos + 8, temp2);
					}
					*(fn_pos + temp1 + temp2) = '\0';
					*(fn_pos + temp1 + temp2 + 1) = '\0';
				}

				memcpy(dir, dir_pos, sizeof(*dir_pos));
				dir_pos++;
				return 1;

			case 0x0f:
				strxcpy(fn_pos + ((((char *)dir_pos)[0] & 0x0f) - 1) * 13,(char *)dir_pos + 1, 10);
				strxcpy(fn_pos + ((((char *)dir_pos)[0] & 0x0f) - 1) * 13 + 5, (char *)dir_pos + 14, 12);
				strxcpy(fn_pos + ((((char *)dir_pos)[0] & 0x0f) - 1) * 13 + 11, (char *)dir_pos + 28, 4);
				break;

			default:
				break;
			}
		}

		dir_pos++;
	}while(*(char *)dir_pos);

L1:
	return -1;
}

static struct fat_dentry *fat_lookup(struct fat_fs *fs, __u32 parent, const char *name)
{
	struct fat_dentry *dir;
	int ret;
	char buf[fs->clus_size];
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
		ret = fat_find_next_file(fs, &parent, sname, dir, buf);
		if (ret < 0)
		{
			DPRINT("can't find file");
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

struct fat_file *fat_open(const char *name, int flags, ...)
{
	struct fat_dentry *dir;
	struct fat_file *fp;
	struct fat_fs *fs;
	struct block_device *bdev;
	char dev_name[MAX_FILE_NAME_LEN];
	int count;

	count = 0;
	while (*name != ':' && *name != '\0')
	{
		dev_name[count] = *name;

		name++;
		count++;

		if (count == MAX_FILE_NAME_LEN)
		{
			DPRINT("%s(): file name length error!\n", __func__);
			return NULL;
		}
	}

	dev_name[count] = '\0';

	if (*name == '\0')
	{
		DPRINT("%s(): Invalid file name \"%s\"!\n", __func__, dev_name);
		return NULL;
	}

	name++;

	bdev = get_bdev_by_name(dev_name);
	fs = bdev->fs;
	dir = fat_lookup(fs, fs->root, name);

	if (!dir)
		return NULL;

	fp = malloc(sizeof(*fp));

	fp->dent = dir;
	fp->offset = 0;
	fp->fs = fs;

	return fp;
}

int fat_close(struct fat_file *fp)
{
	free(fp);
	return 0;
}

int fat_read(struct fat_file *fp, void *buff, size_t size)
{
	__u32 clus_num, clus_size;
	struct fat_fs *fs = fp->fs;
	size_t pos;
	size_t count = 0;
	size_t tmp_size;

	clus_size = fp->fs->clus_size;
	clus_num = fp->dent->clus_hi << 16 | fp->dent->clus_lo;
	pos = fp->offset;

	if (size + pos > fp->dent->size)
	{
		size = fp->dent->size - pos;
	}

	while (pos / clus_size)
	{
		clus_num = fat_get_fat_table(fs, clus_num);
		if (clus_num < 0)
		{
			return -1;
		}
		pos-= clus_size;
	}

	while (size - count > 0)
	{
		if (size + pos - count > clus_size)
		{
			tmp_size = clus_size - pos;
		}
		else
		{
			tmp_size = size - count;
		}

		tmp_size = fat_read_block(fs, (u8 *)buff + count, fs->data + clus_num, pos, tmp_size);
		if (tmp_size < 0)
		{
			break;
		}
		count += tmp_size;
		printf("loading ... %2d\%\r", (count * 100) / size);

		clus_num = fat_get_fat_table(fs, clus_num);
		if (clus_num < 0)
		{
			break;
		}

		pos = 0;
	}

	return count;
}

int fat_write(struct fat_file *fp, const void *buff, size_t size)
{
	return 0;
}
