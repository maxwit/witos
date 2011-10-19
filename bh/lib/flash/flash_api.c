#ifdef CONFIG_DEBUG
#include <stdio.h>
#endif
#include <flash/flash.h>

struct flash_chip *flash_open(const char *name)
{
	struct flash_chip *flash;
	struct block_device *bdev;

	bdev = get_bdev_by_name(name);
	if (!bdev)
		return NULL;

	flash = container_of(bdev, struct flash_chip, bdev);

	flash->callback_func = NULL;
	flash->oob_mode = FLASH_OOB_PLACE;

	return flash;
}

int flash_ioctl(struct flash_chip *flash, int cmd, void *arg)
{
	int ret;

	switch (cmd)
	{
	case FLASH_IOCS_OOB_MODE:
		flash->oob_mode = (OOB_MODE)arg;
		break;

	case FLASH_IOCS_CALLBACK:
	{
		FLASH_CALLBACK *callback;

		callback = (FLASH_CALLBACK *)arg;
		flash->callback_func = callback->func;
		flash->callback_args = callback->args;
		break;
	}

	case FLASH_IOC_SCANBB:
		if (NULL == flash->scan_bad_block)
			return -EINVAL;

		ret = flash->scan_bad_block(flash);

#ifdef CONFIG_DEBUG
		if (ret < 0)
			printf("%s() failed! (errno = %d)\n", __func__, ret);
#endif

		return ret;

	default:
		return -ENOTSUPP;
	}

	return 0;
}

int flash_close(struct flash_chip *flash)
{
	return 0;
}

// fixme
int flash_read(struct flash_chip *flash, void *buff, int start, int count)
{
	int ret;
	u32 ret_len;

	if (FLASH_OOB_RAW == flash->oob_mode)
	{
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
	}
	else
	{
		ret = flash->read(flash, start, count, &ret_len, buff);

		// *start += ret_len;

		if (ret < 0)
		{
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

long flash_write(struct flash_chip *flash, const void *buff, u32 count, u32 ppos)
{
	int ret = 0;
	u32 ret_len;
	struct oob_opt opt;
	u32 size = 0;
	u8 *buff_ptr = (u8 *)buff;

	switch (flash->oob_mode)
	{
	case FLASH_OOB_RAW:
		memset(&opt, 0, sizeof(struct oob_opt));
		opt.op_mode   = FLASH_OOB_RAW;
		opt.data_len  = count;
		opt.data_buff = (u8 *)buff;
		opt.oob_len   = flash->oob_size;

		ret = flash->write_oob(flash, ppos, &opt);

		if (ret < 0)
		{
			DPRINT("%s() failed! ret = %d\n", __func__, ret);
			return ret;
		}
		else if (opt.ret_len != opt.data_len)
		{
			BUG();
		}

		// *ppos += opt.ret_len;

		return opt.ret_len;

	case FLASH_OOB_AUTO:
		if (count % (flash->write_size + flash->oob_size))
		{
			DPRINT("%s(), size invalid!\n", __func__);
			return -EINVAL;
		}

		while (size < count)
		{
			memset(&opt, 0, sizeof(struct oob_opt));
			opt.op_mode   = FLASH_OOB_AUTO;
			opt.data_buff = buff_ptr;
			opt.data_len  = flash->write_size;
			opt.oob_buff  = buff_ptr + flash->write_size;
			opt.oob_len   = flash->oob_size;

			ret = flash->write_oob(flash, ppos, &opt);

			if (ret < 0)
			{
				printf("%s() failed! ret = %d\n", __func__, ret);
				return ret;
			}

			if (opt.ret_len != opt.data_len)
			{
				BUG();
			}

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

int flash_erase(struct flash_chip *flash, u32 start, u32 size, u32 flags)
{
	int ret;
	struct erase_opt opt;

	memset(&opt, 0, sizeof(opt));
	opt.estart = start;
	opt.esize  = size;
	ALIGN_UP(opt.esize, flash->erase_size);
	opt.for_jffs2 = !!(flags & EDF_JFFS2);
	opt.bad_allow = !!(flags & EDF_ALLOWBB);

	ret = flash->erase(flash, &opt);

#ifdef CONFIG_DEBUG
	if (ret < 0)
		printf("%s() failed! (errno = %d)\n", __func__, ret);
#endif

	return ret;
}
