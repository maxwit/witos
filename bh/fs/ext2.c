#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "block.h"
#include "ext2.h"

struct ext2_file_system
{
	struct ext2_super_block sb;
	struct block_device *bdev;
};

static ssize_t ext2_read_block(struct ext2_file_system *fs, int blk_no, size_t off, void *buff, size_t size)
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
	struct ext2_group_desc gde;

	ino--;

	grp_no = ino / sb->s_inodes_per_group;
	ext2_read_block(fs, sb->s_first_data_block + 1, grp_no * sizeof(gde), &gde, sizeof(gde));

	printf("free blocks = %d, free inodes = %d\n", gde.bg_free_blocks_count, gde.bg_free_inodes_count);

	inode = malloc(sb->s_inode_size);

	blk_is = (1 << (sb->s_log_block_size + 10)) / sb->s_inode_size;
	printf("inode size = %d, block size = %d, inodes per block = %d\n",
		sb->s_inode_size, (1 << (sb->s_log_block_size + 10)), blk_is);

	ino_no = ino % sb->s_inodes_per_group;
	// bdev_read_block(bdev, blk_no, ino_no * sb->s_inode_size, inode, sb->s_inode_size);
	ext2_read_block(fs, gde.bg_inode_table + ino_no / blk_is, ino_no * sb->s_inode_size, inode, sb->s_inode_size);

	printf("inode = %d:\n"
		"size = %d, uid = %d, gid = %d, mode = 0x%x\n",
		ino, inode->i_size, inode->i_uid, inode->i_gid, inode->i_mode);

	return inode;
}

struct ext2_dir_entry_2 *ext2_mount(struct block_device *bdev, const char *type)
{
	struct ext2_file_system *fs = malloc(sizeof(*fs));
	struct ext2_super_block *sb = &fs->sb;
	struct ext2_inode *root;
	char buff[DISK_BLOCK_SIZE];

	bdev->get_block(bdev, 2, buff);
	memcpy(sb, buff, sizeof(*sb));

	if (sb->s_magic != 0xef53)
	{
		printf("magic = %x\n", sb->s_magic);
		return NULL;
	}

	printf("label = %s, log block size = %d\n",
		sb->s_volume_name, sb->s_log_block_size);

	fs->bdev = bdev;

	int i = 2;
	// for (i = 1; i < 3; i++)
	{
		root = ext2_read_inode(fs, i);

		char buff[root->i_size];

		ext2_read_block(fs, root->i_block[0], 0, buff, root->i_size);

		int j = 0;
		struct ext2_dir_entry_2 *dentry = (struct ext2_dir_entry_2 *)buff;

		while (j < root->i_size)
		{
			struct ext2_inode *inode;

			if (dentry->rec_len > 0)
			{
				dentry->name[dentry->name_len] = '\0';
				printf("%s: inode = %d, dentry len = %d, sizeof dentry = %ld\n",
					dentry->name, dentry->inode, dentry->rec_len, sizeof(*dentry));

				// read data
				if (dentry->file_type == EXT2_FT_REG_FILE)
				{
					size_t len;

					printf("read regular file %s\n", dentry->name);

					inode = ext2_read_inode(fs, dentry->inode);

					len = ext2_read_block(fs, inode->i_block[0], 0, buff, DISK_BLOCK_SIZE);
					buff[len] = '\0';
					printf("data = %s", buff);
				}

				// if (inode->i_mode)
			}
			else
			{
				break;
			}

			dentry = (struct ext2_dir_entry_2 *)((char *)dentry + dentry->rec_len);

			j += dentry->rec_len;
		}

		printf("\n");
	}

	return NULL;
}
