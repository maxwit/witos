#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include "block.h"
#include "ext2.h"

#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif

#define MAX_MNT_LEN 256

struct ext2_file_system
{
	struct ext2_super_block sb;
	struct block_device *bdev;
	struct ext2_dir_entry_2 *root;
	struct ext2_group_desc *gdt;
};

// TODO:  remove it!
static struct ext2_file_system *g_ext2_fs;

static ssize_t ext2_read_block(struct ext2_file_system *fs, void *buff, int blk_no, size_t off, size_t size)
{
	struct block_device *bdev = fs->bdev;
	struct ext2_super_block *sb = &fs->sb;
	size_t buf_len = (off + size + DISK_BLOCK_SIZE - 1) & ~(DISK_BLOCK_SIZE - 1);
	char blk_buf[buf_len];
	int start_blk = blk_no << (sb->s_log_block_size + 1), cur_blk;

	for (cur_blk = 0; cur_blk < buf_len / DISK_BLOCK_SIZE; cur_blk++)
	{
		bdev->get_block(bdev, start_blk + cur_blk, blk_buf + cur_blk * DISK_BLOCK_SIZE);
	}

	memcpy(buff, blk_buf + off, size);

	return size;
}

static size_t get_start_layer(size_t start_block, size_t index_per_block)
{
	if(start_block < EXT2_IND_BLOCK)
		return 0;
	else if(start_block < EXT2_IND_BLOCK + index_per_block)
		return 1;
	else if(start_block < EXT2_IND_BLOCK + index_per_block +
			index_per_block * index_per_block)
		return 2;
	else
		return 3;
}

static size_t get_ind_block(struct ext2_file_system *fs,
			struct ext2_inode *inode,ssize_t start_block, __le32 block_indexs[], size_t len)
{
	size_t block_size = 1 << (fs->sb.s_log_block_size + 10);
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	size_t i;

	start_block = start_block < 0 ? 0 : start_block;

	if(len <= 0)
		return 0;

	ext2_read_block(fs, buff, inode->i_block[EXT2_IND_BLOCK], 0, block_size);

	for(i = 0; i < len && i < index_per_block - start_block; i++)
	{
		block_indexs[i] = buff[i + start_block];
	}

	return i;
}

static size_t get_dind_block(struct ext2_file_system *fs,
			struct ext2_inode *inode,ssize_t start_block, __le32 block_indexs[], size_t len)
{
	size_t block_size = 1 << (fs->sb.s_log_block_size + 10);
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	__le32 dbuff[index_per_block];
	int i = 0, j, k;

	if(len <= 0)
		return 0;

	ext2_read_block(fs, buff, inode->i_block[EXT2_DIND_BLOCK], 0, block_size);

	start_block = start_block < 0 ? 0 : start_block;

	k = start_block % index_per_block;

	for(j = start_block / index_per_block; j < index_per_block; j++)
	{
		ext2_read_block(fs, dbuff, buff[j], 0, block_size);

		for(; i < len && k < index_per_block; i++, k++)
		{
			block_indexs[i] = dbuff[k];
		}

		if(i >= len)
			return i;

		k = 0;
	}

	return i;
}

static size_t get_tind_block(struct ext2_file_system *fs,
			struct ext2_inode *inode,ssize_t start_block, __le32 block_indexs[], size_t len)
{
	size_t block_size = 1 << (fs->sb.s_log_block_size + 10);
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	__le32 dbuff[index_per_block];
	__le32 tbuff[index_per_block];
	int i, j, k, h;

	if(len <= 0)
		return 0;

	ext2_read_block(fs, buff, inode->i_block[EXT2_TIND_BLOCK], 0, block_size);

	start_block = start_block < 0 ? 0 : start_block;

	h = start_block % index_per_block;

	for(j = start_block / (index_per_block * index_per_block); j < index_per_block; j++)
	{
		ext2_read_block(fs, dbuff, buff[j], 0, block_size);

		for(k = start_block / index_per_block; k < index_per_block; k++)
		{
			ext2_read_block(fs, tbuff, dbuff[k], 0, block_size);

			for(; i < len && h < index_per_block; i++, h++)
			{
				block_indexs[i] = tbuff[h];
			}

			if(i >= len)
				return i;

			h = 0;
		}
	}

	return i;
}

static int get_block_indexs(struct ext2_file_system *fs,
			struct ext2_inode *inode,size_t start_block, __le32 block_indexs[], size_t len)
{
	size_t index_per_block = (1 << (fs->sb.s_log_block_size + 10)) / sizeof(__le32);
	size_t start_layer;
	size_t i = 0;

	if (NULL == inode)
		return -1;

	if(len <= 0)
		return 0;

	start_layer = get_start_layer(start_block, index_per_block);

	if (0 == start_layer)
	{
		for(i = 0; i < len && i < EXT2_IND_BLOCK - start_block; i++)
		{
			block_indexs[i] = inode->i_block[i + start_block];
		}

		start_layer = -1;
	}

	if(i < len && (start_layer == 1 || start_layer == -1))
	{
		i += get_ind_block(fs, inode, start_block - EXT2_IND_BLOCK,
							block_indexs + i, len - i);

		start_layer = -1;
	}

	if(i < len && (start_layer == 2 || start_layer == -1))
	{
		i += get_dind_block(fs, inode, start_block - EXT2_IND_BLOCK - index_per_block
							, block_indexs + i, len - i);

		start_layer = -1;
	}

	if(i < len && (start_layer == 3 || start_layer == -1))
	{
		i += get_tind_block(fs, inode, start_block - EXT2_IND_BLOCK - index_per_block
							- index_per_block * index_per_block, block_indexs + i, len - i);
	}

	return i;
}

static struct ext2_inode *ext2_read_inode(struct ext2_file_system *fs, int ino)
{
	struct ext2_inode *inode;
	struct ext2_super_block *sb = &fs->sb;
	int grp_no, ino_no, blk_no, count;
	struct ext2_group_desc *gde;

	ino--;

	count = (1 << (sb->s_log_block_size + 10)) / sb->s_inode_size;

	grp_no = ino / sb->s_inodes_per_group;
	gde = &fs->gdt[grp_no];

	ino_no = ino % sb->s_inodes_per_group;
	blk_no = ino_no / count;
	ino_no = ino_no % count;

	printf("%s(%d): grp_no = %d, blk_no = %d, ino_no = %d, inode table = %d\n",
		__func__, ino + 1, grp_no, blk_no, ino_no, gde->bg_inode_table);

	inode = malloc(sb->s_inode_size);
	// if

	ext2_read_block(fs, inode, gde->bg_inode_table + blk_no, ino_no * sb->s_inode_size, sb->s_inode_size);

	printf("%s(%d): inode size = %d, uid = %d, gid = %d, mode = 0x%x\n",
		__func__, ino + 1, inode->i_size, inode->i_uid, inode->i_gid, inode->i_mode);

	return inode;
}

struct ext2_file_system *ext2_get_file_system(const char *name)
{
	return g_ext2_fs;
}

struct ext2_dir_entry_2 *ext2_mount(const char *dev_name, const char *path, const char *type)
{
	struct block_device *bdev;
	struct ext2_file_system *fs = malloc(sizeof(*fs));
	struct ext2_super_block *sb = &fs->sb;
	struct ext2_dir_entry_2 *root;
	struct ext2_group_desc *gdt;
	char buff[DISK_BLOCK_SIZE];
	int gdt_num;
	int blk_is;

	bdev = bdev_open(dev_name);
	if (bdev == NULL)
	{
		char str[64];

		sprintf(str, "open(%s)", dev_name);
		perror(str);

		return NULL;
	}

	bdev->get_block(bdev, 2, buff);
	memcpy(sb, buff, sizeof(*sb));

	if (sb->s_magic != 0xef53)
	{
		printf("magic = %x\n", sb->s_magic);
		return NULL;
	}

	blk_is = (1 << (sb->s_log_block_size + 10)) / sb->s_inode_size;

	printf("%s(): label = %s, log block size = %d, "
		"inode size = %d, block size = %d, inodes per block = %d\n",
		__func__, sb->s_volume_name, sb->s_log_block_size,
		sb->s_inode_size, (1 << (sb->s_log_block_size + 10)), blk_is);

	fs->bdev = bdev;

	gdt_num = (sb->s_blocks_count + sb->s_blocks_per_group - 1) / sb->s_blocks_per_group;;
	gdt = malloc(gdt_num * sizeof(struct ext2_group_desc));
	if (NULL == gdt)
	{
		return NULL;
	}

	ext2_read_block(fs, gdt, sb->s_first_data_block + 1, 0, gdt_num * sizeof(struct ext2_group_desc));

	printf("%s(), block group[0 / %d]: free blocks= %d, free inodes = %d\n",
		__func__, gdt_num, gdt->bg_free_blocks_count, gdt->bg_free_inodes_count);

	fs->gdt  = gdt;

	root = malloc(sizeof(*root));
	// if ...

	root->inode = 2;

	fs->root = root;

	g_ext2_fs = fs;

	return root;
}

// fixme: free all resources
int ext2_umount(const char *path)
{
	struct ext2_file_system *fs;

	fs = ext2_get_file_system(path);
	if (NULL == fs)
		return -ENOENT;

	bdev_close(fs->bdev);

	// free fs-> ...

	free(fs);

	return 0;
}

static struct ext2_dir_entry_2 *ext2_lookup(struct ext2_inode *parent, const char *name)
{
	struct ext2_dir_entry_2 *d_match;
	struct ext2_dir_entry_2 *dentry;
	struct ext2_file_system *fs = g_ext2_fs;
	char buff[parent->i_size];
	size_t len = 0, blocks, i;
	size_t block_size = 1 << (fs->sb.s_log_block_size + 10);

	blocks = (parent->i_size + block_size - 1) / block_size;
	__le32 block_indexs[blocks];

	get_block_indexs(fs, parent, 0, block_indexs, blocks);

	for (i = 0; i < blocks; i++)
	{
		ext2_read_block(fs, buff, block_indexs[i], 0, block_size);

		dentry = (struct ext2_dir_entry_2 *)buff;

		while (dentry->rec_len > 0 && len < parent->i_size && len < (i + 1) * block_size)
		{
			dentry->name[dentry->name_len] = '\0';
			printf("%s: inode = %d, dentry size = %d, name size = %d, block = %d\n",
				dentry->name, dentry->inode, dentry->rec_len, dentry->name_len, i);

			if (!strncmp(dentry->name, name, dentry->name_len))
				goto found_entry;

			dentry = (struct ext2_dir_entry_2 *)((char *)dentry + dentry->rec_len);
			len += dentry->rec_len;
		}
	}

	printf("%s(): entry \"%s\" not found!\n",
		__func__, name);

	return NULL;

found_entry:
	d_match = malloc(sizeof(*d_match));
	*d_match = *dentry;

	return d_match;
}

// fixme: to parse path in syntax: "mount_point:filename"
static inline int mnt_of_path(const char *path, char mnt[])
{
	return 0;
}

struct ext2_file *ext2_open(const char *name, int flags, ...)
{
	struct ext2_file_system *fs;
	struct ext2_dir_entry_2 *dir, *de;
	struct ext2_inode *parent;
	struct ext2_file *file;
	char mnt[MAX_MNT_LEN];
	int ret;

	ret =  mnt_of_path(name, mnt);
	// if ...

	fs = ext2_get_file_system(mnt);
	if (NULL == fs)
	{
		printf("\"%s\" not mounted!\n", mnt);
		return NULL;
	}

	dir = fs->root;

	parent = ext2_read_inode(fs, dir->inode);
	//

	name += ret;

	de = ext2_lookup(parent, name);
	if (NULL == de)
		return NULL;

	file = malloc(sizeof(*file));
	// if

	file->pos = 0;
	file->dentry = de;
	file->fs = fs;

	return file;
}

int ext2_close(struct ext2_file *file)
{
	// free(inode, dentry, ...)
	free(file);
	return 0;
}

// fixme: to support "where"
ssize_t ext2_lseek(struct ext2_file *file, ssize_t off, int where)
{
	switch (where)
	{
	default:
		file->pos += off;
		break;
	}

	return file->pos;
}

ssize_t ext2_read(struct ext2_file *file, void *buff, size_t size)
{
	struct ext2_file_system *fs = file->fs;
	struct ext2_inode *inode;
	ssize_t len;
	size_t  blocks;
	size_t  offset, real_size, block_size;
	ssize_t i;

	inode = ext2_read_inode(fs, file->dentry->inode);

	if (file->pos == inode->i_size)
		return 0;

	block_size = 1 << (fs->sb.s_log_block_size + 10);
	char tmp_buff[block_size];

	real_size = min(size, inode->i_size - file->pos);

	blocks = (real_size + (block_size - 1)) / block_size;
	__le32 block_indexs[blocks];

	get_block_indexs(fs, inode, file->pos / block_size, block_indexs, blocks);

	offset = file->pos % block_size;
	len = block_size - offset;

	for(i = 0; i < blocks; i++)
	{
		if(i == blocks - 1)
		{
			len = real_size % block_size == 0 ? block_size : (real_size % block_size);
		}

		ext2_read_block(fs, tmp_buff, block_indexs[i], offset, len);

		memcpy(buff, tmp_buff, len);
		buff += len;
		file->pos += len;
		offset = 0;
		len = block_size;
	}

	return real_size;
}
