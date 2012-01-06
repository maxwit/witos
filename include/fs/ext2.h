#pragma once

#include <fs/fs.h>
#include <fs/ext2_fs.h>

struct ext2_sb_info {
	struct ext2_super_block e2_sb;
	struct ext2_group_desc *gdt;
};

struct ext2_inode_info {
	struct inode vfs_inode;
	// __le32	i_data[EXT2_N_BLOCKS];
	struct ext2_inode *i_e2in;
};

static inline struct ext2_inode_info *EXT2_I(struct inode *inode)
{
	return container_of(inode, struct ext2_inode_info, vfs_inode);
}
