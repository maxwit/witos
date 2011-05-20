#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include "block.h"
#include "ext2.h"

struct ext2_file_system
{
	struct ext2_super_block sb;
	struct block_device *bdev;
	struct ext2_dir_entry_2 *root;
	struct ext2_group_desc *gdt;
} g_ext2_fs;

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

static struct ext2_inode *ext2_read_inode(struct ext2_file_system *fs, int ino)
{
	struct ext2_inode *inode;
	struct ext2_super_block *sb = &fs->sb;
	// struct block_device *bdev = fs->bdev;
	int grp_no, ino_no, blk_is;
	struct ext2_group_desc *gde;

	ino--;

	grp_no = ino / sb->s_inodes_per_group;
	// ext2_read_block(fs, &gde, sb->s_first_data_block + 1, grp_no * sizeof(gde), sizeof(gde));
	gde = &fs->gdt[grp_no];

	inode = malloc(sb->s_inode_size);

	blk_is = (1 << (sb->s_log_block_size + 10)) / sb->s_inode_size;
	ino_no = ino % sb->s_inodes_per_group;

	ext2_read_block(fs, inode, gde->bg_inode_table + ino_no / blk_is, ino_no * sb->s_inode_size, sb->s_inode_size);

	printf("%s(), inode %d:\nsize = %d, uid = %d, gid = %d, mode = 0x%x\n",
		__func__, ino + 1, inode->i_size, inode->i_uid, inode->i_gid, inode->i_mode);

	return inode;
}

struct ext2_file_system *ext2_get_file_system(const char *name)
{
	return &g_ext2_fs;
}

struct ext2_dir_entry_2 *ext2_mount(const char *dev_name, const char *path, const char *type)
{
	struct block_device *bdev;
	struct ext2_file_system *fs = &g_ext2_fs; // malloc(sizeof(*fs));
	struct ext2_super_block *sb = &fs->sb;
	struct ext2_dir_entry_2 *root;
	struct ext2_group_desc *gdt;
	char buff[DISK_BLOCK_SIZE];
	size_t gdt_len;
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

	gdt_len = /* sb->s_blocks_count / sb->s_blocks_per_group * */ 2 * sizeof(struct ext2_group_desc);
	gdt = malloc(gdt_len);
	if (NULL == gdt)
	{
		return NULL;
	}

	ext2_read_block(fs, gdt, sb->s_first_data_block + 1, 0, gdt_len);

	printf("%s(), first block group: free blocks= %d, free inodes = %d\n",
		__func__, gdt->bg_free_blocks_count, gdt->bg_free_inodes_count);

	fs->gdt  = gdt;

	root = malloc(sizeof(*root));
	// if ...

	root->inode = 2;

	fs->root = root;

	return root;
}

int ext2_umount(const char *path)
{
	struct ext2_file_system *fs;

	fs = ext2_get_file_system(path);
	if (NULL == fs)
		return -ENOENT;

	bdev_close(fs->bdev);

	return 0;
}

static struct ext2_dir_entry_2 *ext2_lookup(struct ext2_inode *parent, const char *name)
{
	struct ext2_dir_entry_2 *d_match;
	struct ext2_file_system *fs = &g_ext2_fs;
	char buff[parent->i_size];
	size_t len = 0;

	ext2_read_block(fs, buff, parent->i_block[0], 0, parent->i_size);

	struct ext2_dir_entry_2 *dentry = (struct ext2_dir_entry_2 *)buff;

	while (dentry->rec_len > 0 && len < parent->i_size)
	{
		dentry->name[dentry->name_len] = '\0';
		printf("%s: inode = %d, dentry size = %d, name size = %d\n",
			dentry->name, dentry->inode, dentry->rec_len, dentry->name_len);

		if (!strncmp(dentry->name, name, dentry->name_len))
			goto found_entry;

		dentry = (struct ext2_dir_entry_2 *)((char *)dentry + dentry->rec_len);
		len += dentry->rec_len;
	}

	printf("%s(): entry \"%s\" not found!\n",
		__func__, name);

	return NULL;

found_entry:
	d_match = malloc(sizeof(*d_match));
	*d_match = *dentry;

	return d_match;
}

struct ext2_file *ext2_open(const char *name, int flags, ...)
{
	struct ext2_file_system *fs;
	struct ext2_dir_entry_2 *dir, *de;
	struct ext2_inode *parent;
	struct ext2_file *file;

	fs = ext2_get_file_system(name);
	dir = fs->root;

	parent = ext2_read_inode(fs, dir->inode);
	//

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

#define MIN(x, y) ((x) < (y) ? (x) : (y))

ssize_t ext2_read(struct ext2_file *file, void *buff, size_t size)
{
	struct ext2_file_system *fs = file->fs;
	struct ext2_inode *inode;
	ssize_t len;

	inode = ext2_read_inode(fs, file->dentry->inode);
	char disk_buff[inode->i_size];

	if (file->pos == inode->i_size)
		return 0;

	len = MIN(size, inode->i_size - file->pos);

	len = ext2_read_block(fs, disk_buff, inode->i_block[0], 0, len);
	if (len < 0)
		return len;

	memcpy(buff, disk_buff + file->pos, len);
	file->pos += len;

	return len;
}
