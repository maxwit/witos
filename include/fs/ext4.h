#pragma once

#include <fs.h>
#include <fs/ext4_fs.h>

struct ext4_sb_info {
	struct ext4_super_block e4_sb;
	struct ext4_group_desc *gdt;
};

struct ext4_inode_info {
	struct inode vfs_inode;
	// __le32	i_data[EXT4_N_BLOCKS];
	struct ext4_inode *i_e4in;
};

static inline struct ext4_inode_info *EXT4_I(struct inode *inode)
{
	return container_of(inode, struct ext4_inode_info, vfs_inode);
}
