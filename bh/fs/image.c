#include <image.h>
#include <flash/flash.h>

typedef struct
{
	unsigned chunkId:20;
	unsigned serialNumber:2;
	unsigned byteCount:10;
	unsigned objectId:18;
	unsigned ecc:12;
	unsigned unusedStuff:2;
} YAFFS_TAGS;

typedef union
{
	YAFFS_TAGS asTags;
	__u8 asBytes[8];
} YAFFS_TAGS_UNION;

//fixme: tmp code
static struct nand_oob_layout g_yaffs_oob =
{
	.ecc_code_len = 6,
	.ecc_pos    = {8, 9, 10, 13, 14, 15},
	.free_region =
	{
		{.nOfOffset = 0, .nOfLen = 4},
		{.nOfOffset = 6, .nOfLen = 2},
		{.nOfOffset = 11, .nOfLen = 2}
	}
};

static void yaffs_calc_tags_ecc(YAFFS_TAGS * tags)
{
	unsigned char *b = ((YAFFS_TAGS_UNION *)tags)->asBytes;
	unsigned i, j;
	unsigned ecc = 0;
	unsigned bit = 0;

	tags->ecc = 0;

	for (i = 0; i < 8; i++)
	{
		for (j = 1; j & 0xff; j <<= 1)
		{
			bit++;
			if (b[i] & j)
			{
				ecc ^= bit;
			}
		}
	}

	tags->ecc = ecc;
}

static int yaffs_check_ecc_ontags(YAFFS_TAGS * tags)
{
	unsigned ecc = tags->ecc;

	yaffs_calc_tags_ecc(tags);

	ecc ^= tags->ecc;

	if (ecc && ecc <= 64)
	{
		unsigned char *b = ((YAFFS_TAGS_UNION *)tags)->asBytes;

		ecc--;
		b[ecc / 8] ^= (1 << (ecc & 7));
		yaffs_calc_tags_ecc(tags);

		return 1;
	}
	else if (ecc)
	{
		return -1;
	}

	return 0;
}

static bool check_yaffs1_image(const void *imagebuf)
{
	__u8 free_oob_buff[YAFFS_OOB_SIZE];
	struct oob_free_region *free;
	__u32 offset = 0;
	__u32 bytes = 0;
	int ret;
	__u8 *oob_buf = free_oob_buff;

	memset(free_oob_buff, 0, YAFFS_OOB_SIZE);

	for(free = g_yaffs_oob.free_region; free->nOfLen; free++)
	{
		bytes  = free->nOfLen;
		offset = free->nOfOffset;

		memcpy(oob_buf, imagebuf + offset, bytes);
		oob_buf += bytes;
	}

	ret = yaffs_check_ecc_ontags((YAFFS_TAGS *)free_oob_buff);
	if (ret < 0)
		return false;

	return true;
}

bool check_image_type(PART_TYPE type, const __u8 *image_data)
{
	switch (type)
	{
	case PT_BL_GTH:
		return GTH_MAGIC == *(__u32 *)(image_data + GTH_MAGIC_OFFSET);

	case PT_BL_GBH:
		return GBH_MAGIC == *(__u32 *)(image_data + GBH_MAGIC_OFFSET);

	case PT_BL_GCONF: // fixme
		break;

	case PT_OS_LINUX:
		return LINUX_MAGIC == *(__u32 *)(image_data + LINUX_MAGIC_OFFSET);

	case PT_FS_JFFS2:
		return JFFS2_MAGIC == *(__u16 *)(image_data + JFFS2_MAGIC_OFFSET);

	case PT_FS_YAFFS:
		return check_yaffs1_image(image_data + YAFFS_OOB_SIZE);

	case PT_FS_YAFFS2:
		break;

	case PT_FS_CRAMFS:
		return true; //fixme

	case PT_FS_UBIFS:
		return (UBIFS_MAGIC == *(__u32 *)(image_data + UBIFS_MAGIC_OFFSET));

	default:
		return true; // tmp, fixme!!!
	}

	return true; // fixme!
}
