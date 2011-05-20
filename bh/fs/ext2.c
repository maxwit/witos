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
	
} ext2_fs;

// #define EXT2_BLOCK_TO_SECT(n) ((n) << (sb->s_log_block_size + 1))

static	ssize_t bdev_read_block(struct ext2_file_system *fs, int blk_no, size_t off, void *buff, size_t size)
{
	return bdev_read_block(fs->bdev, blk_no << (fs->sb.s_log_block_size + 1), off, buff, size);
}

static struct ext2_inode *ext2_read_inode(struct ext2_file_system *fs, int ino)
{
	struct ext2_inode *inode;
	struct ext2_super_block *sb = &fs->sb;
	struct block_device *bdev = fs->bdev;
	int blk_no, grp_no, ino_no, blk_is;
	struct ext2_group_desc gde;

	ino--;

	grp_no = ino / sb->s_inodes_per_group;
	blk_no = (sb->s_first_data_block + 1) << (sb->s_log_block_size + 1);
	bdev_read_block(bdev, blk_no, grp_no * sizeof(gde), &gde, sizeof(gde));

	printf("free blocks = %d, free inodes = %d\n", gde.bg_free_blocks_count, gde.bg_free_inodes_count);

	inode = malloc(sb->s_inode_size);

	blk_is = (1 << (sb->s_log_block_size + 10)) / sb->s_inode_size;
	printf("inode size = %d, block size = %d, inodes per block = %d\n",
		sb->s_inode_size, (1 << (sb->s_log_block_size + 10)), blk_is);

	ino_no = ino % sb->s_inodes_per_group;
	blk_no = (gde.bg_inode_table + ino_no / blk_is) << (sb->s_log_block_size + 1);
	bdev_read_block(bdev, blk_no, ino_no * sb->s_inode_size, inode, sb->s_inode_size);

	printf("inode = %d:\n"
		"size = %d, uid = %d, gid = %d, mode = 0x%x\n",
		ino, inode->i_size, inode->i_uid, inode->i_gid, inode->i_mode);

	return inode;
}

struct ext2_dir_entry_2 *ext2_mount(struct block_device *bdev, const char *type)
{
	struct ext2_file_system *fs = &ext2_fs;
	struct ext2_super_block *sb = &fs->sb;
	struct ext2_inode *root;

	bdev_read_block(bdev, 2, 0, sb, sizeof(*sb));

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

		bdev_read_block(bdev, root->i_block[0] << (sb->s_log_block_size + 1), 0, buff, root->i_size);

		int j = 0;
		struct ext2_dir_entry_2 *dentry = (struct ext2_dir_entry_2 *)buff;

		while (j < root->i_size)
		{
			struct ext2_inode *inode;
			
			if (dentry->rec_len > 0)
			{
				dentry->name[dentry->name_len] = '\0';
				printf("%s: inode = %d, dentry len = %d, sizeof dentry = %d\n",
					dentry->name, dentry->inode, dentry->rec_len, sizeof(*dentry));

				// read data
				if (dentry->file_type == EXT2_FT_REG_FILE)
				{
					size_t len;

					printf("read regular file %s\n", dentry->name);

					inode = ext2_read_inode(fs, dentry->inode);

					len = bdev_read_block(bdev, inode->i_block[0] << (sb->s_log_block_size + 1), 0, buff, DISK_BLOCK_SIZE);
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
