#include <block.h>
#include <flash/flash.h>

static inline void blk_buf_init(struct block_buff *blk_buf, void *buff, size_t size)
{
	blk_buf->blk_base = blk_buf->blk_off = buff;
	blk_buf->blk_size = size;
}

static inline PART_TYPE get_image_type(const struct bdev_file *file)
{
#warning
	return PT_FREE;
}

static int flash_bdev_open(struct bdev_file *file, const char *type)
{
	void *buff;
	size_t size;
	struct flash_chip *flash;

	assert(file && file->bdev);

	flash = container_of(file->bdev, struct flash_chip, bdev);

	if (!strcmp(type, "yaffs") || !strcmp(type, "yaffs2"))
		size = (flash->write_size + flash->oob_size) << \
				(flash->erase_shift - flash->write_shift);
	else // fixme!
		size = flash->erase_size;

	buff = malloc(size);
	if (!buff)
		return -ENOMEM;

	blk_buf_init(&file->blk_buf, buff, size);
	file->cur_pos = 0;
	file->img_type = type;

	return 0;
}

static int flash_bdev_close(struct bdev_file *file)
{
	int ret = 0, rest;
	__u32 flash_pos;
	struct block_buff   *blk_buff;
	struct flash_chip   *flash;
	struct block_device *bdev;
	__u32 eflag = EDF_ALLOWBB;

	assert(file && file->bdev);

	bdev = file->bdev;
	flash = container_of(bdev, struct flash_chip, bdev);
	blk_buff = &file->blk_buf;
	rest = blk_buff->blk_off - blk_buff->blk_base;

	if (file->cur_pos > 0 || rest > 0) { // fixme: if mode=write
		int type;
		__u32 pos_adj;

		//printf("%s(): pos = 0x%08x, blk_base = 0x%08x, blk_off = 0x%08x, rest = 0x%08x\n",
			// __func__, file->cur_pos + file->attr->base, blk_buff->blk_base, blk_buff->blk_off, rest);

		type = get_image_type(file);

		switch (type) {
		case PT_FS_YAFFS:
		case PT_FS_YAFFS2:
			pos_adj = file->cur_pos / (flash->write_size + flash->oob_size) * flash->write_size;
			flash_pos = pos_adj;
			ret = flash_erase(flash, flash_pos, bdev->size - pos_adj, eflag);
			break;

#if 0
		case PT_BL_GBIOS:
			flash_pos = base + (CONFIG_GBH_START_BLK << flash->erase_shift) + file->cur_pos;
			ret = flash_erase(flash, flash_pos, rest, eflag);
			break;
#endif
		case PT_FS_JFFS2:
			// eflag |= EDF_JFFS2;
		default: // fixme
			flash_pos = file->cur_pos;
			ret = flash_erase(flash, flash_pos, bdev->size - file->cur_pos, eflag);
			break;
		}

		if (ret < 0) {
			DPRINT("%s(), line %d\n", __func__, __LINE__);
			goto L1;
		}

		if (rest > 0) {
			memset(blk_buff->blk_off, 0xFF, blk_buff->blk_size - rest);

			switch (type) {
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

			if (ret < 0) {
				DPRINT("%s(), line %d\n", __func__, __LINE__);
				goto L1;
			}

			file->cur_pos += rest;
		}
	}

L1:
	free(blk_buff->blk_base);

	return ret < 0 ? ret : 0;
}

static ssize_t flash_bdev_read(struct bdev_file *file, void *buff, __u32 size)
{
	printf("%s() not supported!\n");
	return -EIO;
}

static ssize_t flash_bdev_write(struct bdev_file *file, const void *buff, __u32 size)
{
	int ret = 0;
	__u32 buff_room, flash_pos;
	__u32 eflag = EDF_ALLOWBB;
	PART_TYPE part_type;
	struct flash_chip   *flash;
	struct block_buff   *blk_buff;
	struct block_device *bdev;

	assert(file && file->bdev);

	if (0 == size)
		return 0;

	bdev  = file->bdev;
	flash = container_of(bdev, struct flash_chip, bdev);

	part_type = get_image_type(file);

	if (size + file->cur_pos > bdev->size) {
		char tmp[32];

		val_to_hr_str(bdev->size, tmp);
		printf("File size too large! (> %s)\n", tmp);

		return -ENOMEM;
	}

	blk_buff  = &file->blk_buf;
	buff_room = blk_buff->blk_size - (blk_buff->blk_off - blk_buff->blk_base);

	while (size > 0) {
		if (size >= buff_room) {
			__u32 size_adj;

			memcpy(blk_buff->blk_off, buff, buff_room);

			size -= buff_room;
			buff  = (__u8 *)buff + buff_room;

			buff_room = blk_buff->blk_size;

#ifdef CONFIG_IMAGE_CHECK
			if (file->cur_pos < blk_buff->blk_size) {
				if (false == check_image_type(part_type, blk_buff->blk_base)) {
					printf("\nImage part_type mismatch!"
						"Please check the image name and the target bdev_file!\n");

					return -EINVAL;
				}
			}
#endif

			size_adj = blk_buff->blk_size;

			switch (part_type) {
			case PT_FS_YAFFS:
				// fixme: use macro: RATIO_TO_PAGE(n)
				size_adj = blk_buff->blk_size / (flash->write_size + flash->oob_size) << flash->write_shift;
				flash_pos = file->cur_pos / (flash->write_size + flash->oob_size) << flash->write_shift;
				break;

			case PT_FS_YAFFS2:
				// fixme: use macro: RATIO_TO_PAGE(n)
				size_adj = blk_buff->blk_size / (flash->write_size + flash->oob_size) << flash->write_shift;
				flash_pos = file->cur_pos / (flash->write_size + flash->oob_size) << flash->write_shift;
				break;
#if 0
			case PT_BL_GBIOS:
				flash_pos = base + (CONFIG_GBH_START_BLK << flash->erase_shift) + file->cur_pos;
				break;
#endif
			case PT_FS_JFFS2:
				// eflag |= EDF_JFFS2;
			default:
				flash_pos = file->cur_pos;
				break;
			}

			ret = flash_erase(flash, flash_pos, size_adj, eflag);
			if (ret < 0) {
				printf("%s(), line %d\n", __func__, __LINE__);
				goto L1;
			}

			switch (part_type) {
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

			if (ret < 0) {
				DPRINT("%s(), line %d\n", __func__, __LINE__);
				goto L1;
			}

			file->cur_pos += blk_buff->blk_size;
			blk_buff->blk_off = blk_buff->blk_base;
		} else { // fixme: symi write
			memcpy(blk_buff->blk_off, buff, size);

			blk_buff->blk_off += size;

			size = 0;
		}
	}

L1:
	return ret;
}

#if 0

static struct part_attr *PartGetAttr(struct part_info *pt_info, int nFreeIndex)
{
	if (nFreeIndex > pt_info->parts)
		return NULL;

	return pt_info->attr_tab + nFreeIndex;
}

static bool inline PartProtected(PART_TYPE epart_type)
{
	return PT_BL_GTH == epart_type;
}

int GuPartCreate(struct part_info *pt_info, int nFreeIndex, __u32 size, PART_TYPE part_type)
{
	struct part_attr *pFreeAttr;

	pFreeAttr = PartGetAttr(pt_info, nFreeIndex);
	if (NULL == pFreeAttr)
		return -ENODEV;

	// (size & (host->erase_size - 1)) || // fixme
	if (0 == size || size > pFreeAttr->size)
		return -EINVAL;

	if (PartProtected(part_type))  // reasonable ?
		return -EPERM;

	if (PT_FREE == part_type || PT_FREE != pFreeAttr->part_type)
		return -EINVAL;

	// move to AdjustXXX()?
	if (pt_info->parts == MAX_FLASH_PARTS && size != pFreeAttr->size)
		size = pFreeAttr->size;

	if (size < pFreeAttr->size) {
		struct part_attr *pCurrAttr;

		pCurrAttr = pt_info->attr_tab + pt_info->parts;

		while (pCurrAttr > pFreeAttr) {
			*pCurrAttr = *(pCurrAttr - 1);

			pCurrAttr--;
		}

		pFreeAttr[1].base += size;
		pFreeAttr[1].size -= size;

		pFreeAttr->size = size;

		pt_info->parts++;
	}

	pFreeAttr->part_type = part_type;

	return 0;
}
#endif

#if 0
const char *part_type2str(__u32 type)
{
	switch(type) {
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
#endif

// fixme
int part_show(const struct flash_chip *flash)
{
	int index = 0;
#if 0
	struct part_attr attr_tab[MAX_FLASH_PARTS];
	__u32 nIndex, nRootIndex;
	const char *pBar = "--------------------------------------------------------------------";
	struct linux_config *pLinuxParam;

	if (NULL == flash) {
		printf("ERROR: fail to open a flash!\n");
		return -ENODEV;
	}

	index = part_tab_read(flash, attr_tab, MAX_FLASH_PARTS);

	if (index <= 0) {
		printf("fail to read bdev_file table! (errno = %d)\n", index);
		return index;
	}

	pLinuxParam = sysconf_get_linux_param();

	nRootIndex = pLinuxParam->root_dev;

	printf("\nPartitions on \"%s\":\n", flash->name);
	printf("%s\n", pBar);
	printf(" Index     Start         End     Size       Type        Name\n");
	printf("%s\n", pBar);

	for (nIndex = 0; nIndex < index; nIndex++) {
		char szPartIdx[8], szPartSize[16];

		sprintf(szPartIdx, "%c%d",
			(nIndex == nRootIndex) ? '*' : ' ', nIndex);

		val_to_hr_str(attr_tab[nIndex].size, szPartSize);

		printf("  %-3s   0x%08x - 0x%08x  %-8s  %-9s  \"%s\"\n",
			szPartIdx,
			attr_tab[nIndex].base,
			attr_tab[nIndex].base + attr_tab[nIndex].size,
			szPartSize,
			part_type2str(attr_tab[nIndex].part_type),
			part_get_name(&attr_tab[nIndex]));
	}

	printf("%s\n", pBar);

#endif
	return index;
}

int set_bdev_file_attr(struct bdev_file *file)
{
	char file_attr[CONF_ATTR_LEN];
	char file_val[CONF_VAL_LEN];
	char *pn;

	if (file == NULL) {
		return -1;
	}

	pn = strchr(file->bdev->name, 'p');

	if (pn == NULL) {
		return 0;
	}

	pn++;

	// set file name
	snprintf(file_attr, CONF_ATTR_LEN, "p%c.file.name", *pn);
	if (conf_set_attr(file_attr, file->name) < 0) {
		conf_add_attr(file_attr, file->name);
	}

	// set file size
	snprintf(file_attr, CONF_ATTR_LEN, "p%c.file.size", *pn);
	val_to_dec_str(file_val, file->size);
	if (conf_set_attr(file_attr, file_val) < 0) {
		conf_add_attr(file_attr, file_val);
	}

	return 0;
}

int get_bdev_file_attr(struct bdev_file * file)
{
	char file_attr[CONF_ATTR_LEN];
	char file_val[CONF_VAL_LEN];
	char *pn;

	assert(file != NULL);

	pn = strchr(file->bdev->name, 'p');

	if (pn == NULL) {
		file->name[0] = '\0';
		file->size = 0;
	} else {
		pn++;

		// get file name
		snprintf(file_attr, CONF_ATTR_LEN, "p%c.file.name", *pn);
		if (conf_get_attr(file_attr, file_val) < 0) {
			file->name[0] = '\0';
			file->size = 0;
			return 0;
		} else {
			strncpy(file->name, file_val, MAX_FILE_NAME_LEN);
		}

		// get file size
		snprintf(file_attr, CONF_ATTR_LEN, "p%c.file.size", *pn);
		if (conf_get_attr(file_attr, file_val) < 0 || str_to_val(file_val, &file->size) < 0) {
			file->name[0] = '\0';
			file->size = 0;
		}
	}

	return 0;
}

int flash_fops_init(struct block_device *bdev)
{
	struct bdev_file *file;

	file = zalloc(sizeof(*file)); // fixme: no free
	if (!file)
		return -ENOMEM;

	file->bdev = bdev;
	file->cur_pos = 0;

	file->open  = flash_bdev_open;
	file->read  = flash_bdev_read;
	file->write = flash_bdev_write;
	file->close = flash_bdev_close;

	bdev->file  = file;

	get_bdev_file_attr(file);

	return 0;
}
