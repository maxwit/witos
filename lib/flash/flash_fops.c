#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <fcntl.h>
#include <fs/fs.h>
#include <flash/flash.h>

// fixme
static inline int __flash_read(struct flash_chip *flash, void *buff, int count, int start)
{
	int ret;
	__u32 ret_len;

	if (FLASH_OOB_RAW == flash->oob_mode) {
		struct oob_opt opt;

		memset(&opt, 0, sizeof(struct oob_opt));
		opt.op_mode   = FLASH_OOB_RAW;
		opt.data_buff = buff;
		opt.data_len  = count;

		ret = flash->read_oob(flash, start, &opt);

		// *start += opt.ret_len;

		if (ret < 0)
			return ret;

		return opt.ret_len;
	} else {
		ret = flash->read(flash, start, count, &ret_len, buff);

		// *start += ret_len;

		if (ret < 0) {
#ifdef CONFIG_DEBUG
			if (count != ret_len)
				printf("ERROR: fail to read data! %s()\n", __func__);
			else
				printf("ECC ERROR: while reading data! %s()\n", __func__);
#endif

			return -EFAULT;
		}

		return ret_len;
	}

	return 0;
}

static inline ssize_t __flash_write(struct flash_chip *flash, const void *buff, __u32 count, __u32 ppos)
{
	int ret = 0;
	__u32 ret_len;
	struct oob_opt opt;
	__u32 size = 0;
	__u8 *buff_ptr = (__u8 *)buff;

	switch (flash->oob_mode) {
	case FLASH_OOB_RAW:
		memset(&opt, 0, sizeof(struct oob_opt));
		opt.op_mode   = FLASH_OOB_RAW;
		opt.data_len  = count;
		opt.data_buff = (__u8 *)buff;
		opt.oob_len   = flash->oob_size;

		ret = flash->write_oob(flash, ppos, &opt);

		if (ret < 0) {
			DPRINT("%s() failed! ret = %d\n", __func__, ret);
			return ret;
		} else if (opt.ret_len != opt.data_len) {
			BUG();
		}

		// *ppos += opt.ret_len;

		return opt.ret_len;

	case FLASH_OOB_AUTO:
		if (count % (flash->write_size + flash->oob_size)) {
			DPRINT("%s(), size invalid!\n", __func__);
			return -EINVAL;
		}

		while (size < count) {
			memset(&opt, 0, sizeof(struct oob_opt));
			opt.op_mode   = FLASH_OOB_AUTO;
			opt.data_buff = buff_ptr;
			opt.data_len  = flash->write_size;
			opt.oob_buff  = buff_ptr + flash->write_size;
			opt.oob_len   = flash->oob_size;

			ret = flash->write_oob(flash, ppos, &opt);

			if (ret < 0) {
				printf("%s() failed! ret = %d\n", __func__, ret);
				return ret;
			}

			if (opt.ret_len != opt.data_len)
				BUG();

			ppos += flash->write_size; // + flash->oob_size;

			size += flash->write_size + flash->oob_size;
			buff_ptr += flash->write_size + flash->oob_size;
		}

		return count;

	default:
		if (ppos == flash->chip_size)
			return -ENOSPC;

		if (ppos + count > flash->chip_size)
			count = flash->chip_size - ppos;

		if (!count)
			return 0;

		ret = flash->write(flash, ppos, count, &ret_len, buff);
		// *ppos += ret_len;

		assert(ret == 0);

		return ret_len;
	}

	return 0;
}

static int flash_open(struct file *fp, struct inode *inode)
{
	void *buff;
	size_t size;
	struct flash_chip *flash;
	struct block_buff *blk_buf = &fp->blk_buf;

	flash = NULL; // container_of(inode, struct flash_chip, bdev);
	assert(flash);

	if (fp->flags == O_WRONLY || fp->flags == O_RDWR) {
		if (flash->bdev.flags & BDF_RDONLY) {
			return -EACCES;
		}
	}

	flash->callback_func = NULL;
	flash->oob_mode = FLASH_OOB_PLACE;

	size = (flash->write_size + flash->oob_size) << \
				(flash->erase_shift - flash->write_shift);

	buff = malloc(size);
	if (!buff)
		return -ENOMEM;

	blk_buf->blk_base = blk_buf->blk_off = buff;
	blk_buf->blk_size = blk_buf->max_size = size;

	fp->private_data = flash;

	return 0;
}


static int __flash_erase(struct flash_chip *flash, struct erase_opt *opt)
{
	int ret;
#if 0
	struct erase_opt opt;

	memset(&opt, 0, sizeof(opt));

	opt.estart = start;
	opt.esize  = size;
	opt.flags  = flags;
#endif

	if (opt->esize & (flash->erase_size - 1)) {
#ifdef CONFIG_DEBUG
		size_t size = opt->esize;
#endif

		ALIGN_UP(opt->esize, flash->erase_size);
		GEN_DBG("size (0x%08x) not aligned with flash erase size (0x%08x)!"
			" adjusted to 0x%08x\n",
			size, flash->erase_size, opt->esize);
	}

	ret = flash->erase(flash, opt);

#ifdef CONFIG_DEBUG
	if (ret < 0)
		printf("%s() failed! (errno = %d)\n", __func__, ret);
#endif

	return ret;
}

#if 0
IMG_YAFFS1 -> FLASH_OOB_RAW
IMG_YAFFS2 -> FLASH_OOB_AUTO
normal -> FLASH_OOB_PLACE
#endif

static int flash_ioctl(struct file *fp, int cmd, unsigned long arg)
{
	int ret;
	FLASH_CALLBACK *callback;
	struct flash_chip *flash = fp->private_data;

	switch (cmd) {
	case FLASH_IOCS_OOB_MODE:
		switch (arg) {
		case FLASH_OOB_RAW:
		case FLASH_OOB_AUTO:
			fp->blk_buf.blk_size = (flash->write_size + flash->oob_size) << \
									(flash->erase_shift - flash->write_shift);
			break;

		case FLASH_OOB_PLACE:
			fp->blk_buf.blk_size = flash->erase_size;
			break;

		default:
			return -EINVAL;
		}

		flash->oob_mode = (OOB_MODE)arg;
		break;

	case FLASH_IOC_ERASE:
		return __flash_erase(flash, (struct erase_opt *)arg);

	case FLASH_IOCS_CALLBACK:
		callback = (FLASH_CALLBACK *)arg;
		flash->callback_func = callback->func;
		flash->callback_args = callback->args;
		break;

	case FLASH_IOC_SCANBB:
		if (NULL == flash->scan_bad_block)
			return -EINVAL;

		ret = flash->scan_bad_block(flash);

#ifdef CONFIG_DEBUG
		if (ret < 0)
			printf("%s() failed! (errno = %d)\n", __func__, ret);
#endif

		return ret;

	case FLASH_IOCG_SIZE:
		*(size_t *)arg = flash->bdev.size;

	case FLASH_IOCG_INFO:
		((struct flash_info *)arg)->name       = flash->name;
		((struct flash_info *)arg)->block_size = flash->erase_size;
		((struct flash_info *)arg)->oob_size   = flash->oob_size;
		((struct flash_info *)arg)->page_size  = flash->write_size;
		((struct flash_info *)arg)->type       = flash->type;
		((struct flash_info *)arg)->bdev_base  = flash->bdev.base;
		((struct flash_info *)arg)->bdev_size  = flash->bdev.size;
		((struct flash_info *)arg)->bdev_label = flash->bdev.label;

	default:
		return -ENOTSUPP;
	}

	return 0;
}

static ssize_t flash_read(struct file *fp, void *buff, size_t size, loff_t *off)
{
	struct flash_chip *flash = fp->private_data;

	return __flash_read(flash, buff, size, fp->f_pos);
}

#if 0
	if (fp->f_pos < blk_buff->blk_size) {
		if (false == check_image_type(img_type, blk_buff->blk_base)) {
			printf("\nImage img_type mismatch!"
				"Please check the image name and the target file!\n");

			return -EINVAL;
		}
	}
#endif

static ssize_t flash_write(struct file *fp, const void *buff, size_t size, loff_t *off)
{
	int ret = 0;
	__u32 buff_room, flash_pos;
	struct flash_chip   *flash = fp->private_data;
	struct block_buff   *blk_buff;
	struct block_device *bdev = &flash->bdev;

	// fixme: to be removed
	if (0 == size)
		return 0;

	if (size + fp->f_pos > bdev->size) {
		char tmp[32];

		val_to_hr_str(bdev->size, tmp);
		DPRINT("File size too large! (> %s)\n", tmp);

		return -ENOMEM;
	}

	blk_buff  = &fp->blk_buf;
	buff_room = blk_buff->blk_size - (blk_buff->blk_off - blk_buff->blk_base);

	while (size > 0) {
		if (size >= buff_room) {
			// __u32 size_adj;

			memcpy(blk_buff->blk_off, buff, buff_room);

			size -= buff_room;
			buff  = (__u8 *)buff + buff_room;
			buff_room = blk_buff->blk_size;

#if 1
			switch (flash->oob_mode) {
			case FLASH_OOB_RAW:
			case FLASH_OOB_AUTO:
				// fixme: use macro: RATIO_TO_PAGE(n)
				// size_adj = blk_buff->blk_size / (flash->write_size + flash->oob_size) << flash->write_shift;
				flash_pos = fp->f_pos / (flash->write_size + flash->oob_size) << flash->write_shift;
				break;

			case FLASH_OOB_PLACE:
			default:
				// size_adj = blk_buff->blk_size;
				flash_pos = fp->f_pos;
				break;
			}
#endif

#if 0
		if (flag == AUTO_ERASE)
			ret = flash_erase(flash, flash_pos, size_adj, eflag);
			if (ret < 0) {
				printf("%s(), line %d\n", __func__, __LINE__);
				goto L1;
			}

			switch (img_type) {
			case IMG_YAFFS1:
				ret = flash_ioctl(flash, FLASH_IOCS_OOB_MODE, FLASH_OOB_RAW);
				break;

			case IMG_YAFFS2:
				ret = flash_ioctl(flash, FLASH_IOCS_OOB_MODE, FLASH_OOB_AUTO);
				break;

			default:
				ret = flash_ioctl(flash, FLASH_IOCS_OOB_MODE, FLASH_OOB_PLACE);
				break;
			}
#endif

			ret = __flash_write(flash, blk_buff->blk_base, blk_buff->blk_size, flash_pos);
			if (ret < 0) {
				DPRINT("%s(), line %d\n", __func__, __LINE__);
				goto L1;
			}

			fp->f_pos += blk_buff->blk_size;
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

static int flash_close(struct file *fp)
{
	int ret = 0, rest;
	__u32 flash_pos;
	struct block_buff *blk_buff;
	struct flash_chip *flash = fp->private_data;

	blk_buff = &fp->blk_buf;
	rest = blk_buff->blk_off - blk_buff->blk_base;

	if (rest > 0) {
		//printf("%s(): pos = 0x%08x, blk_base = 0x%08x, blk_off = 0x%08x, rest = 0x%08x\n",
			// __func__, fp->f_pos + fp->attr->base, blk_buff->blk_base, blk_buff->blk_off, rest);

		switch (flash->oob_mode) {
		case FLASH_OOB_RAW:
		case FLASH_OOB_AUTO:
			flash_pos = fp->f_pos / (flash->write_size + flash->oob_size) * flash->write_size;
			break;

		case FLASH_OOB_PLACE:
		default: // fixme
			flash_pos = fp->f_pos;
			break;
		}

		memset(blk_buff->blk_off, 0xFF, rest);

		ret = __flash_write(flash, blk_buff->blk_base, blk_buff->blk_size /* not just the rest */, flash_pos);
		if (ret < 0) {
			DPRINT("%s(), line %d\n", __func__, __LINE__);
			goto L1;
		}

		// fp->f_pos += rest;
	}

L1:
	free(blk_buff->blk_base);

	return ret < 0 ? ret : 0;
}


int set_bdev_file_attr(struct file *fp)
{
#if 0
	char file_attr[CONF_ATTR_LEN];
	char file_val[CONF_VAL_LEN];
	struct block_device *bdev;

	assert(fp != NULL);

	bdev = fp->bdev;

	// set fp name
	snprintf(file_attr, CONF_ATTR_LEN, "bdev.%s.image.name", bdev->name);
	if (conf_set_attr(file_attr, fp->name) < 0) {
		conf_add_attr(file_attr, fp->name);
	}

	// set fp size
	snprintf(file_attr, CONF_ATTR_LEN, "bdev.%s.image.size", bdev->name);
	val_to_dec_str(file_val, fp->size);
	if (conf_set_attr(file_attr, file_val) < 0) {
		conf_add_attr(file_attr, file_val);
	}
#endif
	return 0;
}

const struct file_operations flash_fops = {
	.open  = flash_open,
	.read  = flash_read,
	.write = flash_write,
	.close = flash_close,
	.ioctl = flash_ioctl,
};

int flash_fops_init(struct block_device *bdev)
{
	bdev->fops = &flash_fops;
	return 0;
}
