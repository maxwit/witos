#include <sysconf.h>
#include <flash/flash.h>
#include <flash/part.h>

static int g_home_index = -1; // to be removed
static int g_curr_index = -1;

static __INLINE__ struct partition *get_flash_part(struct flash_chip *flash, int num)
{
	return flash->part_tab + num;
}

static struct partition *get_part(int index)
{
	int base_index = 0, parts;
	int flash_num;
	struct flash_chip *flash;
	struct partition *part;

	if (index < 0)
		return NULL;

	if (PART_CURR == index)
		index = g_curr_index;

	for (flash_num = 0; (flash = flash_get(flash_num)); flash_num++)
	{
		parts = flash->pt_info->parts;

		if (index >= base_index && index < base_index + parts)
		{
			part = get_flash_part(flash, index - base_index);

			return part;
		}

		base_index += parts;
	}

	return NULL;
}

struct partition *part_open(int index, OP_MODE mode)
{
	struct flash_chip   *flash;
	struct partition   *part;
	struct part_attr   *attr;
	struct block_buff  *blk_buff;

	part = get_part(index);

	if (NULL == part || NULL == part->attr)
		return NULL;

	flash    = part->host;
	attr     = part->attr;
	blk_buff = &part->blk_buf;

	switch (attr->part_type)
	{
	case PT_FS_YAFFS:
	case PT_FS_YAFFS2:
		blk_buff->blk_size = (flash->write_size + flash->oob_size) << (flash->erase_shift - flash->write_shift);
		break;

#if 0
	case PT_BL_GBH:
		blk_buff->blk_size = attr->part_size;
		break;
#endif

#ifndef CONFIG_GTH_WRITE
	case PT_BL_GTH:
		if (OP_RDWR == mode || OP_WRONLY == mode)
			return NULL;
#endif

	default:
		blk_buff->blk_size = flash->erase_size;
		break;
	}

	blk_buff->blk_off = blk_buff->blk_base = malloc(blk_buff->blk_size);

	part->cur_pos = 0;

	return part;
}

int part_close(struct partition *part)
{
	int ret = 0, rest;
	u32 flash_pos;
	struct part_attr  *attr;
	struct block_buff *blk_buff;
	struct flash_chip *flash;
	u32 dwEraseFlag = EDF_ALLOWBB;

	if (NULL == part)
		return -EINVAL;

	flash = part->host;
	BUG_ON(NULL == flash);

	attr = part->attr;

	blk_buff = &part->blk_buf;

	rest = blk_buff->blk_off - blk_buff->blk_base;

	if (part->cur_pos > 0 || rest > 0) // fixme: if mode=write
	{
		int type;
		u32 part_size, part_base, pos_adj;

		//printf("%s(): pos = 0x%08x, blk_base = 0x%08x, blk_off = 0x%08x, rest = 0x%08x\n",
			// __func__, part->cur_pos + part->attr->part_base, blk_buff->blk_base, blk_buff->blk_off, rest);

		type = part_get_type(part);
		part_base = part_get_base(part);
		part_size = part_get_size(part);

		switch (type)
		{
		case PT_FS_YAFFS:
		case PT_FS_YAFFS2:
			pos_adj = part->cur_pos / (flash->write_size + flash->oob_size) * flash->write_size;
			flash_pos = part_base + pos_adj;
			ret = flash_erase(flash, flash_pos, part_size - pos_adj, dwEraseFlag);
			break;

#if 0
		case PT_BL_GBIOS:
			flash_pos = part_base + (CONFIG_GBH_START_BLK << flash->erase_shift) + part->cur_pos;
			ret = flash_erase(flash, flash_pos, rest, dwEraseFlag);
			break;
#endif
		case PT_FS_JFFS2:
			// dwEraseFlag |= EDF_JFFS2;
		default: // fixme
			flash_pos = part_base + part->cur_pos;
			ret = flash_erase(flash, flash_pos, part_size - part->cur_pos, dwEraseFlag);
			break;
		}

		if (ret < 0)
		{
			DPRINT("%s(), line %d\n", __func__, __LINE__);
			goto L1;
		}

		if (rest > 0)
		{
			memset(blk_buff->blk_off, 0xFF, blk_buff->blk_size - rest);

			switch (type)
			{
			case PT_FS_YAFFS:
				ret = flash_ioctl(flash, FLASH_IOCS_OOB_MODE, (void *)FLASH_OOB_RAW);
				ret = flash_write(flash, blk_buff->blk_base, rest, flash_pos);
				break;

			case PT_FS_YAFFS2:
				ret = flash_ioctl(flash, FLASH_IOCS_OOB_MODE, (void *)FLASH_OOB_AUTO);
				ret = flash_write(flash, blk_buff->blk_base, blk_buff->blk_size, flash_pos);
				break;

			default:
				flash_ioctl(flash, FLASH_IOCS_OOB_MODE, (void *)FLASH_OOB_PLACE);
				ret = flash_write(flash, blk_buff->blk_base, rest, flash_pos);
				break;
			}

			if (ret < 0)
			{
				DPRINT("%s(), line %d\n", __func__, __LINE__);
				goto L1;
			}

			part->cur_pos += rest;
		}
	}

L1:
	free(blk_buff->blk_base);

	return ret < 0 ? ret : 0;
}

long part_read(struct partition *part, void *buff, u32 size)
{
	printf("%s() not supported!\n");
	return -EIO;
}

long part_write(struct partition *part, const void *buff, u32 size)
{
	int ret = 0;
	u32 buff_room, flash_pos;
	u32 part_base;
	u32 dwEraseFlag = EDF_ALLOWBB;
	PART_TYPE part_type;
	struct flash_chip *flash;
	struct part_attr   *attr;
	struct block_buff  *blk_buff;

	if (NULL == part || NULL == buff)
	{
		return -EINVAL;
	}

	if (0 == size)
	{
		return 0;
	}

	flash = part->host;
	BUG_ON(NULL == flash);

	part_type = part_get_type(part);
	part_base = part_get_base(part);

	attr = part->attr;

	if (size + part->cur_pos > attr->part_size)
	{
		char tmp[32];

		val_to_hr_str(attr->part_size, tmp);
		printf("File size too large! (> %s)\n", tmp);

		return -ENOMEM;
	}

	blk_buff    = &part->blk_buf;
	buff_room = blk_buff->blk_size - (blk_buff->blk_off - blk_buff->blk_base);

	while (size > 0)
	{
		if (size >= buff_room)
		{
			u32 len_adj;

			memcpy(blk_buff->blk_off, buff, buff_room);

			size -= buff_room;
			buff  = (u8 *)buff + buff_room;

			buff_room = blk_buff->blk_size;

#ifdef CONFIG_IMAGE_CHECK
			if (part->cur_pos < blk_buff->blk_size)
			{
				if (FALSE == check_image_type(part_type, blk_buff->blk_base))
				{
					printf("\nImage part_type mismatch!"
						"Please check the image name and the target partition!\n");

					return -EINVAL;
				}
			}
#endif

			len_adj = blk_buff->blk_size;

			switch (part_type)
			{
			case PT_FS_YAFFS:
				// fixme: use macro: RATIO_TO_PAGE(n)
				len_adj   = blk_buff->blk_size / (flash->write_size + flash->oob_size) << flash->write_shift;
				flash_pos = part_base + (part->cur_pos / (flash->write_size + flash->oob_size) << flash->write_shift);
				break;

			case PT_FS_YAFFS2:
				// fixme: use macro: RATIO_TO_PAGE(n)
				len_adj   = blk_buff->blk_size / (flash->write_size + flash->oob_size) << flash->write_shift;
				flash_pos = part_base + (part->cur_pos / (flash->write_size + flash->oob_size) << flash->write_shift);
				break;
#if 0
			case PT_BL_GBIOS:
				flash_pos = part_base + (CONFIG_GBH_START_BLK << flash->erase_shift) + part->cur_pos;
				break;
#endif
			case PT_FS_JFFS2:
				// dwEraseFlag |= EDF_JFFS2;
			default:
				flash_pos = part_base + part->cur_pos;
				break;
			}

			ret = flash_erase(flash, flash_pos, len_adj, dwEraseFlag);
			if (ret < 0)
			{
				printf("%s(), line %d\n", __func__, __LINE__);
				goto L1;
			}

			switch (part_type)
			{
			case PT_FS_YAFFS:
				ret = flash_ioctl(flash, FLASH_IOCS_OOB_MODE, (void *)FLASH_OOB_RAW);
				ret = flash_write(flash, blk_buff->blk_base, blk_buff->blk_size, flash_pos);
				break;

			case PT_FS_YAFFS2:
				ret = flash_ioctl(flash, FLASH_IOCS_OOB_MODE, (void *)FLASH_OOB_AUTO);
				ret = flash_write(flash, blk_buff->blk_base, blk_buff->blk_size, flash_pos);
				break;

			default:
				ret = flash_ioctl(flash, FLASH_IOCS_OOB_MODE, (void *)FLASH_OOB_PLACE);
				ret = flash_write(flash, blk_buff->blk_base, blk_buff->blk_size, flash_pos);
				break;
			}

			if (ret < 0)
			{
				DPRINT("%s(), line %d\n", __func__, __LINE__);
				goto L1;
			}

			part->cur_pos += blk_buff->blk_size;
			blk_buff->blk_off = blk_buff->blk_base;
		}
		else // fixme: symi write
		{
			memcpy(blk_buff->blk_off, buff, size);

			blk_buff->blk_off += size;

			size = 0;
		}
	}

L1:
	return ret;
}

int part_get_index(const struct partition *part)
{
	struct flash_chip *flash;

	flash = part->host;

	BUG_ON(NULL == flash);

	return part - flash->part_tab;
}

#if 0

static struct part_attr *PartGetAttr(struct part_info *pt_info, int nFreeIndex)
{
	if (nFreeIndex > pt_info->parts)
	{
		return NULL;
	}

	return pt_info->attr_tab + nFreeIndex;
}

static BOOL __INLINE__ PartProtected(PART_TYPE epart_type)
{
	return PT_BL_GTH == epart_type;
}

int GuPartCreate(struct part_info *pt_info, int nFreeIndex, u32 size, PART_TYPE part_type)
{
	struct part_attr *pFreeAttr;

	pFreeAttr = PartGetAttr(pt_info, nFreeIndex);
	if (NULL == pFreeAttr)
	{
		return -ENODEV;
	}

	// (size & (host->erase_size - 1)) || // fixme
	if (0 == size || size > pFreeAttr->part_size)
	{
		return -EINVAL;
	}

	if (PartProtected(part_type))  // reasonable ?
	{
		return -EPERM;
	}

	if (PT_FREE == part_type || PT_FREE != pFreeAttr->part_type)
	{
		return -EINVAL;
	}

	// move to AdjustXXX()?
	if (pt_info->parts == MAX_FLASH_PARTS && size != pFreeAttr->part_size)
		size = pFreeAttr->part_size;

	if (size < pFreeAttr->part_size)
	{
		struct part_attr *pCurrAttr;

		pCurrAttr = pt_info->attr_tab + pt_info->parts;

		while (pCurrAttr > pFreeAttr)
		{
			*pCurrAttr = *(pCurrAttr - 1);

			pCurrAttr--;
		}

		pFreeAttr[1].part_base += size;
		pFreeAttr[1].part_size -= size;

		pFreeAttr->part_size = size;

		pt_info->parts++;
	}

	pFreeAttr->part_type = part_type;

	return 0;
}
#endif

int part_change(int index)
{
	if (NULL == get_part(index))
		return -ENODEV;

	g_curr_index = index;

	return g_curr_index;
}

int part_tab_read(const struct flash_chip *host, struct part_attr attr_tab[], int index)
{
	u32 nIndex;
	struct part_info *pt_info = host->pt_info;

	if (NULL == pt_info)
		return -EINVAL;

	if (index > pt_info->parts)
		index = pt_info->parts;

	for (nIndex = 0; nIndex < index; nIndex++)
		attr_tab[nIndex] = pt_info->attr_tab[nIndex];

	// fixme: shall we keep the data consistant with FLASH

	return index;
}

int part_get_image(const struct partition *part, char name[], u32 *size)
{
	struct image_info *image = part->image;

	if (image->image_size && name)
	{
		strncpy(name, image->image_name, MAX_FILE_NAME_LEN);
	}

	if (size)
	{
		*size = image->image_size;
	}

	return image->image_size;
}

int part_set_image(struct partition *part, char name[], u32 size)
{
	struct image_info *image = part->image;

	strncpy(image->image_name, name, MAX_FILE_NAME_LEN);
	image->image_size = size;

	return image->image_size;
}

void part_set_home(int index)
{
	g_home_index = index;
}

int part_get_home(void)
{
	return g_home_index;
}

//
const char *part_type2str(u32 type)
{
	switch(type)
	{
	case PT_FREE:
		return PT_STR_FREE;

	case PT_BL_GTH:
		return PT_STR_GB_TH;

	case PT_BL_GBH:
		return PT_STR_GB_BH;

	case PT_BL_GCONF:
		return PT_STR_GB_CONF;

	case PT_OS_LINUX:
		return PT_STR_LINUX;

	case PT_OS_WINCE:
		return PT_STR_WINCE;

	case PT_FS_RAMDISK:
		return PT_STR_RAMDISK;

	case PT_FS_CRAMFS:
		return PT_STR_CRAMFS;

	case PT_FS_JFFS2:
		return PT_STR_JFFS2;

	case PT_FS_YAFFS:
		return PT_STR_YAFFS;

	case PT_FS_YAFFS2:
		return PT_STR_YAFFS2;

	case PT_FS_UBIFS:
		return PT_STR_UBIFS;

	default:
		return "Unknown";
	}
}

const char *part_get_name(const struct part_attr *attr)
{
	if (attr->part_name[0] == '\0')
	 	return part_type2str(attr->part_type);

	return attr->part_name;
}

// fixme
int part_show(const struct flash_chip *flash)
{
	int index;
	struct part_attr attr_tab[MAX_FLASH_PARTS];
	u32 nIndex, nRootIndex;
	const char *pBar = "--------------------------------------------------------------------";
	struct linux_config *pLinuxParam;

	if (NULL == flash)
	{
		printf("ERROR: fail to open a flash!\n");
		return -ENODEV;
	}

	index = part_tab_read(flash, attr_tab, MAX_FLASH_PARTS);

	if (index <= 0)
	{
		printf("fail to read partition table! (errno = %d)\n", index);
		return index;
	}

	pLinuxParam = sysconf_get_linux_param();

	nRootIndex = pLinuxParam->root_dev;

	printf("\nPartitions on \"%s\":\n", flash->name);
	printf("%s\n", pBar);
	printf(" Index     Start         End     Size       Type        Name\n");
	printf("%s\n", pBar);

	for (nIndex = 0; nIndex < index; nIndex++)
	{
		char szPartIdx[8], szPartSize[16];

		sprintf(szPartIdx, "%c%d",
			(nIndex == nRootIndex) ? '*' : ' ', nIndex);

		val_to_hr_str(attr_tab[nIndex].part_size, szPartSize);

		printf("  %-3s   0x%08x - 0x%08x  %-8s  %-9s  \"%s\"\n",
			szPartIdx,
			attr_tab[nIndex].part_base,
			attr_tab[nIndex].part_base + attr_tab[nIndex].part_size,
			szPartSize,
			part_type2str(attr_tab[nIndex].part_type),
			part_get_name(&attr_tab[nIndex]));
	}

	printf("%s\n", pBar);

	return index;
}

