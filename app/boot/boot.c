#include <g-bios.h>
#include <sysconf.h>
#include <fs.h>
#include <net/tftp.h>
#include <uart/uart.h>
#include <flash/part.h>
#include "atag.h"

#define LINUX_IMAGE_NAME "zImage"
#define CONFIG_RAM_BANK_NUM  1 // fixme!

typedef void (*LINUX_KERNEL_ENTRY)(int, int, u32);


static struct tag *begin_setup_atag (void *tag_base)
{
	struct tag *cur_tag;

	cur_tag = (struct tag *)tag_base;

	cur_tag->hdr.tag = ATAG_CORE;
	cur_tag->hdr.size = tag_size(tag_core);

	cur_tag->u.core.flags    = 0;
	cur_tag->u.core.pagesize = 0;
	cur_tag->u.core.rootdev  = 0;

	return cur_tag;
}

static struct tag *setup_cmdline_atag(struct tag *cur_tag, char *cmd_line)
{
	cur_tag = tag_next(cur_tag);

	while (*cmd_line && ' ' == *cmd_line)
		cmd_line++;

	cur_tag->hdr.tag = ATAG_CMDLINE;
	cur_tag->hdr.size = (sizeof(struct tag_header) + strlen(cmd_line) + 3) >> 2;

	strcpy(cur_tag->u.cmdline.cmdline, cmd_line);

	return cur_tag;
}


// fixme
static struct tag *setup_mem_atag (struct tag *cur_tag)
{
	int i;

	for (i = 0; i < CONFIG_RAM_BANK_NUM; i++)
	{
		cur_tag = tag_next(cur_tag);

		cur_tag->hdr.tag  = ATAG_MEM;
		cur_tag->hdr.size = tag_size(tag_mem32);

		cur_tag->u.mem.start = SDRAM_BASE;
		cur_tag->u.mem.size  = SDRAM_SIZE;
	}

	return cur_tag;
}


#if 0
static struct tag *setup_ramdisk_atag(struct tag *cur_tag, struct image_cache *rd_cache)
{
	cur_tag = tag_next(cur_tag);

	cur_tag->hdr.tag = ATAG_RAMDISK;
	cur_tag->hdr.size = tag_size(TagRamDisk);
	cur_tag->stRamDisk.flags = 3; // fixme!!
	cur_tag->stRamDisk.nStart = (u32)rd_cache->cache_base;
	cur_tag->stRamDisk.size  = rd_cache->cache_size;

	return cur_tag;
}
#endif


static struct tag *setup_initrd_atag(struct tag *cur_tag, struct image_cache *initrd_cache)
{
	cur_tag = tag_next(cur_tag);

	cur_tag->hdr.tag  = ATAG_INITRD2;
	cur_tag->hdr.size = tag_size(tag_initrd);
	cur_tag->u.initrd.start = (u32)initrd_cache->cache_base;
	cur_tag->u.initrd.size  = initrd_cache->cache_size;

	return cur_tag;
}


static void end_setup_atag (struct tag *cur_tag)
{
	cur_tag = tag_next(cur_tag);

	cur_tag->hdr.tag = ATAG_NONE;
	cur_tag->hdr.size = 0;
}

// fixme
static int boot_usage(void)
{
	printf("Usage: boot [options] \n"
		"\noptions:\n"
		"\t-t [kernel_image_name]:  download kernel image via tftp\n"
		"\t-f [<part_num>]:  root=/dev/mtdblock<part_num>\n"
		"\t-n [nfs_server:/path/to/nfs/root]:  boot via NFS\n"
		"\t-l [kernel command line]:  boot via spcified CmdLine\n"
		"\t-r [ramdisk_image_name]:  boot with Ramdisk\n"
		"\t-c <console_name, baudrate>:  set console name and baudrate\n"
		"\t-m <machine ID>:  set board machine ID\n"
		"\t-v:  print the linux parameters and quit\n"
		"\nExamples:\n"
		"\t..."
		);

	return 0;
}


static int part_load_image(PART_TYPE type, struct image_cache **ppImgCache)
{
	int  index;
	u32  offset;
	int  part_num;
	int  errno = 0;
	char err_msg[64];
	char image_name[MAX_FILE_NAME_LEN];
	u32  image_size;
	struct part_attr   attr_tab[MAX_FLASH_PARTS];
	struct flash_chip  *flash;
	struct partition   *part;
	struct image_cache *img_cache;


	printf("Loading %s image from flash ... ",
		type == PT_OS_LINUX ? "kernel" : "ramdisk");

	img_cache = image_cache_get(type);
	BUG_ON (NULL == img_cache || NULL == img_cache->cache_base);

	flash = flash_open(BOOT_FLASH_ID);
	if (!flash)
	{
		strcpy(err_msg, "cannot open flash");
		errno = -ENODEV;
		goto L0;
	}

	part_num = part_tab_read(flash, attr_tab, MAX_FLASH_PARTS);
	if (part_num < 0)
	{
		strcpy(err_msg, "cannot read partition table");
		goto L1;
	}

	for (index = 0; index < part_num && attr_tab[index].part_type != type; ++index);

	if (index == part_num)
	{
		strcpy(err_msg, "cannot find the partition");
		errno = -1;
		goto L1;
	}

	part = part_open(index, OP_RDONLY);
	if (NULL == part)
	{
		strcpy(err_msg, "cannot open partition");
		errno = -1;
		goto L1;
	}

	errno = part_get_image(part, image_name, &image_size);
#if 0
	if (errno < 0)
	{
		strcpy(err_msg, "no image, pls download it first");
		errno = -ENOENT;
		goto L2;
	}
#endif

	// fixme
	if (image_size <= 0 || image_size >= part_get_size(part))
	{
		printf("warning: image size invalid(%d/0x%08x), setting to partition size(%d/0x%08x)!\n",
			image_size, image_size, part_get_size(part), part_get_size(part));

		image_size = part_get_size(part);
	}

	offset = part_get_base(part);

	errno = flash_read(flash, img_cache->cache_base, offset, image_size);
	if (errno < 0)
	{
		strcpy(err_msg, "flash read failed");
		goto L2;
	}

	img_cache->cache_size = image_size;

	*ppImgCache = img_cache;

L2:
	part_close(part);
L1:
	flash_close(flash);
L0:
	if (errno >= 0)
	{
		puts("OK.");
	}
	else
	{
		printf("failed! (%s)\n", err_msg);
	}

	return errno;
}


static int tftp_load_image(PART_TYPE type, char image_name[], struct image_cache **ppImgCache)
{
	int ret;
	struct tftp_opt dlopt;
	struct image_cache *img_cache;


	printf("Downloading %s image via tftp:\n",
		type == PT_OS_LINUX ? "linux" : "ramdisk");

	img_cache = image_cache_get(type);
	BUG_ON (NULL == img_cache || NULL == img_cache->cache_base);

	memset(&dlopt, 0x0, sizeof(dlopt));

	net_get_server_ip(&dlopt.server_ip);
	strcpy(dlopt.file_name, image_name);
	dlopt.load_addr = img_cache->cache_base;

	ret = net_tftp_load(&dlopt);
	if (ret < 0)
	{
		printf("fail to download %s!\n", image_name);
		goto L1;
	}

	img_cache->cache_size = ret;

	*ppImgCache = img_cache;

L1:
	return ret;
}


// fixme: for debug stage while no flash driver available
static int build_command_line(char *cmd_line, size_t max_len)
{
	int ret = 0;
	char *str = cmd_line;
	struct part_info    *part_conf;
	struct net_config   *net_conf;
	struct linux_config *linux_conf;
	struct flash_chip       *flash;
	char local_ip[IPV4_STR_LEN], server_ip[IPV4_STR_LEN], net_mask[IPV4_STR_LEN];
	char mtd_dev[MTD_DEV_NAME_LEN];
	char part_list[512];
	int part_num;
	int mtd_root = 0, root_idx;
	struct part_attr attr_tab[MAX_FLASH_PARTS];

	memset(cmd_line, 0, max_len);

	sysconf_get_part_info(&part_conf);
	sysconf_get_net_info(&net_conf);
	sysconf_get_linux_param(&linux_conf);

	ip_to_str(local_ip, net_conf->local_ip);
	ip_to_str(server_ip, net_conf->server_ip);
	ip_to_str(net_mask, net_conf->net_mask);

	root_idx = linux_conf->root_dev;

	flash = flash_open(BOOT_FLASH_ID);
	if (NULL == flash)
	{
		ret = -ENODEV;
		printf("fail to open flash %d\n", BOOT_FLASH_ID);
		goto L1;
	}

	ret = flash_get_mtd_name(flash, mtd_dev, sizeof(mtd_dev));
	if (ret < 0)
	{
		printf("fail to get mtd name!\n");
		goto L2;
	}

	// MARK_MAXWIT_TRAINING: C Basics {
	part_num = part_tab_read(flash, attr_tab, MAX_FLASH_PARTS);

	if (part_num < 0)
	{
		printf("fail to read part table! (ret = %d)\n", part_num);
		ret = part_num;
		goto L2;
	}
	else if (mtd_dev[0] != '\0' && part_num > 0)
	{
		u32 part_idx;
		char *part_str = part_list;

		part_str += sprintf(part_str, "mtdparts=%s:", mtd_dev);

		for (part_idx = 0; part_idx < part_num; part_idx++)
		{
			struct part_attr *attr = attr_tab + part_idx;

			switch (attr->part_type)
			{
			default:
#ifdef CONFIG_FS_PARTS_ONLY
				break;
#elif defined(CONFIG_MERGE_GB_PARTS)
			case PT_BL_GTH:
				gb_base = attr->part_base;
				gb_size += attr->part_size;
				break;

			case PT_BL_GBH:
				if (gb_base > attr->part_base)
				{
					gb_base = attr->part_base;
				}
				gb_size += attr->part_size;
				break;

			case PT_BL_GB_CONF: // must be defined and the the last GB partition
				if (gb_base > attr->part_base)
				{
					gb_base = attr->part_base;
				}
				gb_size += attr->part_size;

				part_str += sprintf(part_str, "0x%x@0x%x(g-bios),", gb_size, gb_base);
				mtd_root++;

				break;
#endif

			case PT_FS_JFFS2:
			case PT_FS_YAFFS:
			case PT_FS_YAFFS2:
			case PT_FS_CRAMFS:
			case PT_FS_UBIFS:
				part_str += sprintf(part_str, "0x%x@0x%x(%s),",
								attr->part_size,
								attr->part_base,
								attr->part_name);

				if (part_idx < root_idx)
					mtd_root++;

				break;

			case PT_BAD:
				break;
			}
		}

		*--part_str = '\0';
	}
	// } MARK_MAXWIT_TRAINING

	BUG_ON(root_idx >= part_num);

	if (linux_conf->boot_mode & BM_NFS)
	{
		str += sprintf(str, "root=/dev/nfs rw nfsroot=%s:%s",
					server_ip,
					linux_conf->nfs_path);
	}
	else if (linux_conf->boot_mode & BM_FLASHDISK)
	{
		int nRootType = attr_tab[root_idx].part_type;

		switch (nRootType)
		{
		case PT_FS_CRAMFS:
		case PT_FS_JFFS2:
		case PT_FS_YAFFS:
		case PT_FS_YAFFS2:
			str += sprintf(str, "root=/dev/mtdblock%d rw rootfstype=%s", // fixme
						mtd_root, part_type2str(nRootType));
			break;
		case PT_FS_UBIFS:
			str += sprintf(str, "ubi.mtd=%d root=ubi0_0 rootfstype=ubifs", root_idx);
			break;
		default:
			ret = -EINVAL;
			printf("partition %d (%s) is NOT set for filesystem!\n",
				root_idx, attr_tab[root_idx].part_name);
			goto L2;
		}
	}

	str += sprintf(str, " %s", part_list);

// MARK_MAXWIT_TRAINING: Linux Operation {
#ifdef CONFIG_DHCP
	str += sprintf(str, " ip=dhcp");
#else
	str += sprintf(str, " ip=%s:%s:%s:%s:maxwit.googlecode.com:eth0:off",
				local_ip, server_ip, server_ip, net_mask);
#endif
// } MARK_MAXWIT_TRAINING

	str += sprintf(str, " console=%s", linux_conf->console_device);

L2:
	flash_close(flash);
L1:
	return ret;
}

static int show_boot_args(void *tag_base)
{
	int i;
	struct tag *arm_tag = tag_base;


	if (arm_tag->hdr.tag != ATAG_CORE)
	{
		printf("Invalid ATAG pointer (0x%08x)!\n", arm_tag);
		return -EINVAL;
	}

	i = 0;
	printf("\n");

	while (1)
	{
		printf("[ATAG %d] ", i);

		switch (arm_tag->hdr.tag)
		{
		case ATAG_CORE:
			printf("****** ATAG Begin ******\n");
			break;

		case ATAG_CMDLINE:
			printf("Kernel command line:\n%s\n", arm_tag->u.cmdline.cmdline);
			// printf("Kernel command line\n");
			break;

		case ATAG_MEM:
			printf("Memory: [0x%08x - 0x%08x]\n",
				arm_tag->u.mem.start, arm_tag->u.mem.start + arm_tag->u.mem.size);
			break;

		case ATAG_INITRD2:
			printf("Initrd: [0x%08x - 0x%08x]\n",
				arm_tag->u.initrd.start, arm_tag->u.initrd.start + arm_tag->u.initrd.size);
			break;

		case ATAG_NONE:
			printf("******  ATAG End  ******\n");
			return 0;

		default: // fixme
			printf("arm_tag = 0x%08x\n", arm_tag->hdr.tag);
			break;
		}

		arm_tag = tag_next(arm_tag);
		i++;
	}

	return 0;
}


int main(int argc, char *argv[])
{
	int  i, ret = 0;
	u32  dev_num;
	char cmd_line[DEFAULT_KCMDLINE_LEN];
	BOOL show_args = FALSE;
	struct image_cache *pImgKernel, *rd_cache;
	struct linux_config *linux_conf;
	struct net_config   *net_conf;
	struct tag *arm_tag;
	LINUX_KERNEL_ENTRY exec_linux;

	sysconf_get_linux_param(&linux_conf);
	sysconf_get_net_info(&net_conf);

	// fixme
	for (i = 1; i < argc; i++)
	{
		char *arg = argv[i];

		if ('-' != arg[0] || strlen(arg) != 2)
		{
			boot_usage();
			return -EINVAL;
		}

		switch (arg[1])
		{
		case 't':
			if (i + 1 == argc || '-' == argv[i + 1][0])
			{
				linux_conf->kernel_image[0] = '\0';
				break;
			}

			strcpy(linux_conf->kernel_image, argv[++i]);

			break;

		case 'r':
			linux_conf->boot_mode = BM_RAMDISK;

			if (i + 1 == argc || '-' == argv[i + 1][0])
			{
				linux_conf->ramdisk_image[0] = '\0';
				break;
			}

			strcpy(linux_conf->ramdisk_image, argv[++i]);

			break;

		case 'f':
			linux_conf->boot_mode = BM_FLASHDISK;

			if (i + 1 == argc || '-' == argv[i + 1][0])
				break;

			if (string2value(argv[++i], &dev_num) < 0)
			{
				printf("Invalid partition number (%s)!\n", argv[i]);
				break;
			}

			// fixme: if num > nParts
			linux_conf->root_dev = dev_num;

			break;

		case 'n':
			linux_conf->boot_mode = BM_NFS;

			if (i + 1 == argc || '-' == argv[i + 1][0])
				break;

			arg = argv[++i];

			while (*arg && *arg != ':' && *arg != '/') arg++;

			switch (*arg)
			{
			case ':':
				*arg = '\0';
				strcpy(linux_conf->nfs_path, arg + 1);

			case '\0':
				if (str_to_ip((u8 *)&net_conf->server_ip, argv[i]) < 0)
					printf("wrong ip format! (%s)\n", argv[i]);

				break;

			default:
				strcpy(linux_conf->nfs_path, argv[i]);
				break;
			}

			break;

		case 'm':
			if (i + 1 == argc || '-' == argv[i + 1][0])
			{
				printf("Invalid option: %s, current MachID = %u\n", argv[i], linux_conf->mach_id);
				boot_usage();
				return -ENAVAIL;
			}

			string2value(argv[++i], (u32 *)&linux_conf->mach_id);

			break;

		case 'c':
			if (i + 1 == argc || '-' == argv[i + 1][0])
			{
				printf("Invalid option: %s", argv[i]);
				boot_usage();
				return -EINVAL;
			}

			strcpy(linux_conf->console_device, argv[++i]);

			break;

		case 'v':
			show_args = TRUE;
			break;

		case 'l':
			if (i + 1 == argc)
			{
				//fix me:need to check kernel command line.
				if (linux_conf->cmd_line[0] != 'r')
				{
					printf("Invalid kernel command line!\n");
					return -EINVAL;
				}
			}
			else
			{
				strcpy(linux_conf->cmd_line, argv[++i]);

				for (i++; i < argc; i++)
				{
					strcat(linux_conf->cmd_line, " ");
					strcat(linux_conf->cmd_line, argv[i]);
				}
			}

			linux_conf->boot_mode = BM_CMDLINE;
			break;

		case 'h':
			boot_usage();
			return 0;

		default:
			printf("Invalid option: \"%s\"!\n", arg);
			boot_usage();
			return -EINVAL;
		}
	}

	if (argc > 2 || (2 == argc && FALSE == show_args))
	{
		sysconf_save();
	}

	if(!(linux_conf->boot_mode & BM_CMDLINE))
	{
		ret = build_command_line(cmd_line, sizeof(cmd_line));
		if (ret < 0)
		{
			printf("Warning: fail to setup linux command line!\n\"%s\"\n", cmd_line);
			// anyway, we'd go ahead :)
		}
	}
	else
	{
		strncpy(cmd_line, linux_conf->cmd_line, DEFAULT_KCMDLINE_LEN);
	}


	BUG_ON ((linux_conf->boot_mode & ~BM_MASK) == 0);

	if (!show_args)
	{
		if (linux_conf->kernel_image[0] != '\0') // fixme for error return (i.e. permission denied)
		{
			ret = tftp_load_image(PT_OS_LINUX, linux_conf->kernel_image, &pImgKernel);
		}
		else
		{
			ret = part_load_image(PT_OS_LINUX, &pImgKernel);
		}
	}

	// TODO: check the kernel image

	if (ret < 0)
	{
		goto L1;
	}

	exec_linux = (LINUX_KERNEL_ENTRY)pImgKernel->cache_base;

	printf("\n");

	arm_tag = begin_setup_atag((void *)ATAG_BASE);
	arm_tag = setup_cmdline_atag(arm_tag, cmd_line);

	if (linux_conf->boot_mode & BM_RAMDISK)
	{
		if (!show_args)
		{
			if (linux_conf->ramdisk_image[0] != '\0')
			{
				ret = tftp_load_image(PT_FS_RAMDISK, linux_conf->ramdisk_image, &rd_cache);
			}
			else
			{
				ret = part_load_image(PT_FS_RAMDISK, &rd_cache);
			}

			if (ret < 0)
			{
				goto L1;
			}
		}

		// TODO: check the ramdisk

		arm_tag = setup_initrd_atag(arm_tag, rd_cache);
	}

	arm_tag = setup_mem_atag(arm_tag);

	end_setup_atag(arm_tag);

	if (show_args)
	{
		show_boot_args((void *)ATAG_BASE);
		return 0;
	}

	irq_disable(); // fixme

	exec_linux(0, linux_conf->mach_id, ATAG_BASE);

L1:
	printf("\n");

	return ret;
}


void __INIT__ auto_boot_linux(void)
{
	int time_out = 3;
	char *argv[1] = {"boot"};

	while (1)
	{
		int index;

		printf("\rAutoboot after %d seconds. Press any key to interrupt.", time_out);

		if (0 == time_out)
		{
			break;
		}

		for (index = 0; index < 10; index++)
		{
			mdelay(100);

			if (uart_rxbuf_count())
			{
				puts("\n");
				return;
			}
		}

		time_out--;
	}

	puts("\n");

	main(1, argv); // fixme
}

