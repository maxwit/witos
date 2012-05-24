#include <errno.h>
#include <stdio.h>
#include <fs.h>

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

static bool system_supports_ext2()
{
	return file_system_type_get("ext2") == NULL ? false : true;
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
