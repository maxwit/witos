#include <errno.h>
#include <stdio.h>
#include <fs.h>

typedef unsigned long long __le64;

/* for s_flags */
#define EXT2_FLAGS_TEST_FILESYS		0x0004

/* for s_feature_compat */
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL		0x0004

/* for s_feature_ro_compat */
#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER	0x0001
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE	0x0002
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR	0x0004
#define EXT4_FEATURE_RO_COMPAT_HUGE_FILE	0x0008
#define EXT4_FEATURE_RO_COMPAT_GDT_CSUM		0x0010
#define EXT4_FEATURE_RO_COMPAT_DIR_NLINK	0x0020
#define EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE	0x0040

/* for s_feature_incompat */
#define EXT2_FEATURE_INCOMPAT_FILETYPE		0x0002
#define EXT3_FEATURE_INCOMPAT_RECOVER		0x0004
#define EXT3_FEATURE_INCOMPAT_JOURNAL_DEV	0x0008
#define EXT2_FEATURE_INCOMPAT_META_BG		0x0010
#define EXT4_FEATURE_INCOMPAT_EXTENTS		0x0040 /* extents support */
#define EXT4_FEATURE_INCOMPAT_64BIT		0x0080
#define EXT4_FEATURE_INCOMPAT_MMP		0x0100
#define EXT4_FEATURE_INCOMPAT_FLEX_BG		0x0200

#define EXT2_FEATURE_RO_COMPAT_SUPP	(EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER| \
					 EXT2_FEATURE_RO_COMPAT_LARGE_FILE| \
					 EXT2_FEATURE_RO_COMPAT_BTREE_DIR)
#define EXT2_FEATURE_INCOMPAT_SUPP	(EXT2_FEATURE_INCOMPAT_FILETYPE| \
					 EXT2_FEATURE_INCOMPAT_META_BG)
#define EXT2_FEATURE_INCOMPAT_UNSUPPORTED	~EXT2_FEATURE_INCOMPAT_SUPP
#define EXT2_FEATURE_RO_COMPAT_UNSUPPORTED	~EXT2_FEATURE_RO_COMPAT_SUPP

#define EXT3_FEATURE_RO_COMPAT_SUPP	(EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER| \
					 EXT2_FEATURE_RO_COMPAT_LARGE_FILE| \
					 EXT2_FEATURE_RO_COMPAT_BTREE_DIR)
#define EXT3_FEATURE_INCOMPAT_SUPP	(EXT2_FEATURE_INCOMPAT_FILETYPE| \
					 EXT3_FEATURE_INCOMPAT_RECOVER| \
					 EXT2_FEATURE_INCOMPAT_META_BG)
#define EXT3_FEATURE_INCOMPAT_UNSUPPORTED	~EXT3_FEATURE_INCOMPAT_SUPP
#define EXT3_FEATURE_RO_COMPAT_UNSUPPORTED	~EXT3_FEATURE_RO_COMPAT_SUPP

/*
 * Structure of the super block
 */
struct ext_super_block {
/*00*/	__le32	s_inodes_count;		/* Inodes count */
	__le32	s_blocks_count_lo;	/* Blocks count */
	__le32	s_r_blocks_count_lo;	/* Reserved blocks count */
	__le32	s_free_blocks_count_lo;	/* Free blocks count */
/*10*/	__le32	s_free_inodes_count;	/* Free inodes count */
	__le32	s_first_data_block;	/* First Data Block */
	__le32	s_log_block_size;	/* Block size */
	__le32	s_log_cluster_size;	/* Allocation cluster size */
/*20*/	__le32	s_blocks_per_group;	/* # Blocks per group */
	__le32	s_clusters_per_group;	/* # Clusters per group */
	__le32	s_inodes_per_group;	/* # Inodes per group */
	__le32	s_mtime;		/* Mount time */
/*30*/	__le32	s_wtime;		/* Write time */
	__le16	s_mnt_count;		/* Mount count */
	__le16	s_max_mnt_count;	/* Maximal mount count */
	__le16	s_magic;		/* Magic signature */
	__le16	s_state;		/* File system state */
	__le16	s_errors;		/* Behaviour when detecting errors */
	__le16	s_minor_rev_level;	/* minor revision level */
/*40*/	__le32	s_lastcheck;		/* time of last check */
	__le32	s_checkinterval;	/* max. time between checks */
	__le32	s_creator_os;		/* OS */
	__le32	s_rev_level;		/* Revision level */
/*50*/	__le16	s_def_resuid;		/* Default uid for reserved blocks */
	__le16	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT4_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 *
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	__le32	s_first_ino;		/* First non-reserved inode */
	__le16  s_inode_size;		/* size of inode structure */
	__le16	s_block_group_nr;	/* block group # of this superblock */
	__le32	s_feature_compat;	/* compatible feature set */
/*60*/	__le32	s_feature_incompat;	/* incompatible feature set */
	__le32	s_feature_ro_compat;	/* readonly-compatible feature set */
/*68*/	__u8	s_uuid[16];		/* 128-bit uuid for volume */
/*78*/	char	s_volume_name[16];	/* volume name */
/*88*/	char	s_last_mounted[64];	/* directory where last mounted */
/*C8*/	__le32	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT4_FEATURE_COMPAT_DIR_PREALLOC flag is on.
	 */
	__u8	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	__u8	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	__le16	s_reserved_gdt_blocks;	/* Per group desc for online growth */
	/*
	 * Journaling support valid if EXT4_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
/*D0*/	__u8	s_journal_uuid[16];	/* uuid of journal superblock */
/*E0*/	__le32	s_journal_inum;		/* inode number of journal file */
	__le32	s_journal_dev;		/* device number of journal file */
	__le32	s_last_orphan;		/* start of list of inodes to delete */
	__le32	s_hash_seed[4];		/* HTREE hash seed */
	__u8	s_def_hash_version;	/* Default hash version to use */
	__u8	s_jnl_backup_type;
	__le16  s_desc_size;		/* size of group descriptor */
/*100*/	__le32	s_default_mount_opts;
	__le32	s_first_meta_bg;	/* First metablock block group */
	__le32	s_mkfs_time;		/* When the filesystem was created */
	__le32	s_jnl_blocks[17];	/* Backup of the journal inode */
	/* 64bit support valid if EXT4_FEATURE_COMPAT_64BIT */
/*150*/	__le32	s_blocks_count_hi;	/* Blocks count */
	__le32	s_r_blocks_count_hi;	/* Reserved blocks count */
	__le32	s_free_blocks_count_hi;	/* Free blocks count */
	__le16	s_min_extra_isize;	/* All inodes have at least # bytes */
	__le16	s_want_extra_isize; 	/* New inodes should reserve # bytes */
	__le32	s_flags;		/* Miscellaneous flags */
	__le16  s_raid_stride;		/* RAID stride */
	__le16  s_mmp_update_interval;  /* # seconds to wait in MMP checking */
	__le64  s_mmp_block;            /* Block for multi-mount protection */
	__le32  s_raid_stripe_width;    /* blocks on all data disks (N*stride)*/
	__u8	s_log_groups_per_flex;  /* FLEX_BG group size */
	__u8	s_reserved_char_pad;
	__le16  s_reserved_pad;
	__le64	s_kbytes_written;	/* nr of lifetime kilobytes written */
	__le32	s_snapshot_inum;	/* Inode number of active snapshot */
	__le32	s_snapshot_id;		/* sequential ID of active snapshot */
	__le64	s_snapshot_r_blocks_count; /* reserved blocks for active
					      snapshot's future use */
	__le32	s_snapshot_list;	/* inode number of the head of the
					   on-disk snapshot list */
#define EXT4_S_ERR_START offsetof(struct ext4_super_block, s_error_count)
	__le32	s_error_count;		/* number of fs errors */
	__le32	s_first_error_time;	/* first time an error happened */
	__le32	s_first_error_ino;	/* inode involved in first error */
	__le64	s_first_error_block;	/* block involved of first error */
	__u8	s_first_error_func[32];	/* function where the error happened */
	__le32	s_first_error_line;	/* line number where error happened */
	__le32	s_last_error_time;	/* most recent time of an error */
	__le32	s_last_error_ino;	/* inode involved in last error */
	__le32	s_last_error_line;	/* line number where error happened */
	__le64	s_last_error_block;	/* block involved of last error */
	__u8	s_last_error_func[32];	/* function where the error happened */
#define EXT4_S_ERR_END offsetof(struct ext4_super_block, s_mount_opts)
	__u8	s_mount_opts[64];
	__le32	s_usr_quota_inum;	/* inode for tracking user quota */
	__le32	s_grp_quota_inum;	/* inode for tracking group quota */
	__le32	s_overhead_clusters;	/* overhead blocks/clusters in fs */
	__le32  s_reserved[109];        /* Padding to the end of the block */
};


static bool system_supports_ext2()
{
	return file_system_type_get("ext2") == NULL ? false : true;
}

void ext_sb_list(struct ext_super_block *esb)
{
	printf("s_blocks_count_lo = 0x%x\n", esb->s_blocks_count_lo);
	printf("s_r_blocks_count_lo = 0x%x\n", esb->s_r_blocks_count_lo);
	printf("s_free_blocks_count_lo = 0x%x\n", esb->s_free_blocks_count_lo);
	printf("s_first_data_block = 0x%x\n", esb->s_first_data_block);
	printf("s_log_block_size = 0x%x\n", esb->s_log_block_size);
	printf("s_log_cluster_size = 0x%x\n", esb->s_log_cluster_size);
	printf("s_clusters_per_group = 0x%x\n", esb->s_clusters_per_group);
	printf("s_inodes_per_group = 0x%x\n", esb->s_inodes_per_group);
	printf("s_mtime = 0x%x\n", esb->s_mtime);
	printf("s_mnt_count = 0x%x\n", esb->s_mnt_count);
	printf("s_max_mnt_count = 0x%x\n", esb->s_max_mnt_count);
	printf("s_magic = 0x%x\n", esb->s_magic);
	printf("s_state = 0x%x\n", esb->s_state);
	printf("s_errors = 0x%x\n", esb->s_errors);
	printf("s_minor_rev_level = 0x%x\n", esb->s_minor_rev_level);
	printf("s_checkinterval = 0x%x\n", esb->s_checkinterval);
	printf("s_creator_os = 0x%x\n", esb->s_creator_os);
	printf("s_rev_level = 0x%x\n", esb->s_rev_level);
	printf("s_def_resgid = 0x%x\n", esb->s_def_resgid);
	printf("s_first_ino = 0x%x\n", esb->s_first_ino);
	printf("s_inode_size = 0x%x\n", esb->s_inode_size);
	printf("s_block_group_nr = 0x%x\n", esb->s_block_group_nr);
	printf("s_feature_compat = 0x%x\n", esb->s_feature_compat);
	printf("s_feature_incompat = 0x%x\n", esb->s_feature_incompat);
	printf("s_feature_ro_compat = 0x%x\n", esb->s_feature_ro_compat);
	printf("s_prealloc_blocks = 0x%x\n", esb->s_prealloc_blocks);
	printf("s_prealloc_dir_blocks = 0x%x\n", esb->s_prealloc_dir_blocks);
	printf("s_reserved_gdt_blocks = 0x%x\n", esb->s_reserved_gdt_blocks);
	printf("s_journal_dev = 0x%x\n", esb->s_journal_dev);
	printf("s_last_orphan = 0x%x\n", esb->s_last_orphan);
//	printf("s_hash_seed[4] = 0x%x\n", esb->s_hash_seed[4]);
	printf("s_def_hash_version = 0x%x\n", esb->s_def_hash_version);
	printf("s_jnl_backup_type = 0x%x\n", esb->s_jnl_backup_type);
	printf("s_desc_size = 0x%x\n", esb->s_desc_size);
	printf("s_default_mount_opts = 0x%x\n", esb->s_default_mount_opts);
	printf("s_first_meta_bg = 0x%x\n", esb->s_first_meta_bg);
	printf("s_mkfs_time = 0x%x\n", esb->s_mkfs_time);
//	printf("s_jnl_blocks[17] = 0x%x\n", esb->s_jnl_blocks[17]);
	printf("s_r_blocks_count_hi = 0x%x\n", esb->s_r_blocks_count_hi);
	printf("s_free_blocks_count_hi = 0x%x\n", esb->s_free_blocks_count_hi);
	printf("s_min_extra_isize = 0x%x\n", esb->s_min_extra_isize);
	printf("s_want_extra_isize = 0x%x\n", esb->s_want_extra_isize);
	printf("s_flags = 0x%x\n", esb->s_flags);
	printf("s_raid_stride = 0x%x\n", esb->s_raid_stride);
	printf("s_mmp_update_interval = 0x%x\n", esb->s_mmp_update_interval);
	printf("s_mmp_block = 0x%x\n", esb->s_mmp_block);
	printf("s_raid_stripe_width = 0x%x\n", esb->s_raid_stripe_width);
	printf("s_log_groups_per_flex = 0x%x\n", esb->s_log_groups_per_flex);
	printf("s_reserved_char_pad = 0x%x\n", esb->s_reserved_char_pad);
	printf("s_reserved_pad = 0x%x\n", esb->s_reserved_pad);
	printf("s_kbytes_written = 0x%x\n", esb->s_kbytes_written);
	printf("s_snapshot_inum = 0x%x\n", esb->s_snapshot_inum);
	printf("s_snapshot_id = 0x%x\n", esb->s_snapshot_id);
	printf("s_snapshot_r_blocks_count = 0x%x\n", esb->s_snapshot_r_blocks_count);
	printf("s_snapshot_list = 0x%x\n", esb->s_snapshot_list);
	printf("s_error_count = 0x%x\n", esb->s_error_count);
	printf("s_first_error_time = 0x%x\n", esb->s_first_error_time);
	printf("s_first_error_ino = 0x%x\n", esb->s_first_error_ino);
	printf("s_first_error_block = 0x%x\n", esb->s_first_error_block);
//	printf("s_first_error_func[32] = 0x%x\n", esb->s_first_error_func[32]);
	printf("s_first_error_line = 0x%x\n", esb->s_first_error_line);
	printf("s_last_error_time = 0x%x\n", esb->s_last_error_time);
	printf("s_last_error_ino = 0x%x\n", esb->s_last_error_ino);
	printf("s_last_error_line = 0x%x\n", esb->s_last_error_line);
	printf("s_last_error_block = 0x%x\n", esb->s_last_error_block);
//	printf("s_last_error_func[32] = 0x%x\n", esb->s_last_error_func[32]);
//	printf("s_mount_opts[64] = 0x%x\n", esb->s_mount_opts[64]);
	printf("s_usr_quota_inum = 0x%x\n", esb->s_usr_quota_inum);
	printf("s_grp_quota_inum = 0x%x\n", esb->s_grp_quota_inum);
	printf("s_overhead_clusters = 0x%x\n", esb->s_overhead_clusters);
//	printf("s_reserved[109] = 0x%x\n", esb->s_reserved[109]);
}

int ck_ext2_feature(uint32_t fc, uint32_t frc, uint32_t fi)
{

	/* Distinguish between ext3 and ext2 */
	if (fc & EXT3_FEATURE_COMPAT_HAS_JOURNAL) {
		DPRINT("fc = 0x%x\n", fc);
		return -ENOTSUPP;
	}

	/* Any features which ext2 doesn't understand */
	if ((frc & EXT2_FEATURE_RO_COMPAT_UNSUPPORTED) ||
	    (fi  & EXT2_FEATURE_INCOMPAT_UNSUPPORTED)) {
		DPRINT("frc = 0x%x, fi = 0x%x\n", frc, fi);
		return -ENOTSUPP;
	}

	return 0;
}

int ck_ext3_feature(uint32_t fc, uint32_t frc, uint32_t fi)
{

	/* ext3 requires journal */
	if (!(fc & EXT3_FEATURE_COMPAT_HAS_JOURNAL))
		return -ENOTSUPP;

	/* Any features which ext3 doesn't understand */
	if ((frc & EXT3_FEATURE_RO_COMPAT_UNSUPPORTED) ||
	    (fi  & EXT3_FEATURE_INCOMPAT_UNSUPPORTED))
		return -ENOTSUPP;

	return 0;
}

int ck_ext4_feature(uint32_t fc, uint32_t frc, uint32_t fi)
{
	/* Distinguish from jbd */
	if (fi & EXT3_FEATURE_INCOMPAT_JOURNAL_DEV)
		return -ENOTSUPP;

	/*
	 * If the filesystem does not have a journal and ext2 is not
	 * present, then force this to be detected as an ext2
	 * filesystem.
	 */
	if (!(fc & EXT3_FEATURE_COMPAT_HAS_JOURNAL) &&
	    !system_supports_ext2())
		return 0;

	/* Ext4 has at least one feature which ext3 doesn't understand */
	if (!(frc & EXT3_FEATURE_RO_COMPAT_UNSUPPORTED) &&
	    !(fi  & EXT3_FEATURE_INCOMPAT_UNSUPPORTED))
		return -ENOTSUPP;

	return 0;
}
