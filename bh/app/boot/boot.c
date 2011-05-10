#include <sysconf.h>
#include <getopt.h>
#include <image.h>
#include <net/tftp.h>
#include <uart/uart.h>
#include <flash/part.h>
#include "atag.h"

#define LINUX_IMAGE_NAME     "zImage"
#define CONFIG_RAM_BANK_NUM  1 // fixme!
#define UIMAGE_HEAD_SIZE     64

typedef void (*LINUX_KERNEL_ENTRY)(int, int, u32);

//static char app_option[][CMD_OPTION_LEN] = {"t", "r", "f", "n", "m", "c", "v", "l", "0"};

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

static struct tag *setup_initrd_atag(struct tag *cur_tag, void *image_buff, u32 image_size)
{
	cur_tag = tag_next(cur_tag);

	cur_tag->hdr.tag  = ATAG_INITRD2;
	cur_tag->hdr.size = tag_size(tag_initrd);
	cur_tag->u.initrd.start = (u32)image_buff;
	cur_tag->u.initrd.size  = image_size;

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

static int part_load_image(PART_TYPE type, u8 **buff_ptr, u32 *buff_len)
{
	int  index;
	u32  offset;
	int  part_num;
	int  ret;
	char image_name[MAX_FILE_NAME_LEN];
	u32  image_size, part_size;
	struct part_attr   attr_tab[MAX_FLASH_PARTS];
	struct flash_chip  *flash;
	struct partition   *part;
	u8 *buff;

	printf("Loading %s image from flash ... ",
		type == PT_OS_LINUX ? "kernel" : "ramdisk");

	flash = flash_open(BOOT_FLASH_ID);
	if (!flash)
	{
		ret = -ENODEV;
		goto L0;
	}

	part_num = part_tab_read(flash, attr_tab, MAX_FLASH_PARTS);
	if (part_num < 0)
	{
		ret = -EIO;
		goto L1;
	}

	for (index = 0; index < part_num && attr_tab[index].part_type != type; ++index);

	if (index == part_num)
	{
		ret = -1;
		goto L1;
	}

	part = part_open(index, OP_RDONLY);
	if (NULL == part)
	{
		ret = -1;
		goto L1;
	}

	offset = part_get_base(part);

	ret = part_get_image(part, image_name, &image_size);

	// to be optimized
	if (ret < 0 || image_size <= 0 || image_size >= part_get_size(part))
	{
		// if (ret < 0) {}

		part_size = part_get_size(part);

		printf("%s(): Image seems invalid, still try to load.\n"
			"Current image size = %d/0x%08x, adjusting to partition size (%d/0x%08x)!\n",
			__func__, image_size, image_size, part_size, part_size);

		image_size = part_size;
	}

	buff = malloc(image_size);
	if (!buff)
	{
		ret = -ENOMEM;
		goto L2;
	}

	ret = flash_read(flash, buff, offset, image_size);
	if (ret < 0)
	{
		ret = -EIO;
		goto L2;
	}

	*buff_ptr = buff;
	*buff_len = image_size;

L2:
	part_close(part);
L1:
	flash_close(flash);
L0:

	return ret;
}

static int tftp_load_image(PART_TYPE type, char image_name[], u8 **buff_ptr, u32 *buff_len)
{
	int ret;
	struct tftp_opt dlopt;

	printf("Downloading %s image via tftp:\n",
		type == PT_OS_LINUX ? "linux" : "ramdisk");

	memset(&dlopt, 0x0, sizeof(dlopt));

	net_get_server_ip(&dlopt.server_ip);
	strcpy(dlopt.file_name, image_name);
	dlopt.load_addr = malloc(MB(3)); //fixme!!!
	if (!dlopt.load_addr)
	{
		printf("%s(): fail to malloc buffer!\n", __func__);
		return -ENOMEM;
	}

	ret = net_tftp_load(&dlopt);
	if (ret < 0)
	{
		printf("fail to download %s!\n", image_name);
		goto L1;
	}

	*buff_ptr = dlopt.load_addr;
	*buff_len = ret;
L1:
	return ret;
}

// fixme: for debug stage while no flash driver available
static int build_command_line(char *cmd_line, size_t max_len)
{
	int ret = 0;
	char *str = cmd_line;
	struct image_info   *image_conf;
	struct net_config   *net_conf;
	struct linux_config *linux_conf;
	struct flash_chip       *flash;
	char local_ip[IPV4_STR_LEN], server_ip[IPV4_STR_LEN], net_mask[IPV4_STR_LEN];
	const char *mtd_dev;
	char part_list[512], *part_str = part_list;
	int part_num;
	int mtd_root = 0, root_idx;
	struct part_attr attr_tab[MAX_FLASH_PARTS];
	u32 part_idx;
#ifdef CONFIG_MERGE_GB_PARTS
	u32 gb_base, gb_size;
#endif
	u32 client_ip, client_mask; // fixme

	memset(cmd_line, 0, max_len);

	image_conf = sysconf_get_image_info();
	net_conf = sysconf_get_net_info();
	linux_conf = sysconf_get_linux_param();

	ndev_ioctl(NULL, NIOC_GET_IP, &client_ip);
	ndev_ioctl(NULL, NIOC_GET_MASK, &client_mask);

	ip_to_str(local_ip, client_ip);
	ip_to_str(server_ip, net_conf->server_ip);
	ip_to_str(net_mask, client_mask);

	root_idx = linux_conf->root_dev;

	flash = flash_open(BOOT_FLASH_ID);
	if (NULL == flash)
	{
		ret = -ENODEV;
		printf("fail to open flash %d\n", BOOT_FLASH_ID);
		goto L1;
	}

	mtd_dev = flash_get_mtd_name(flash);
	if (mtd_dev == NULL)
	{
		printf("fail to get mtd name!\n");
		goto L2;
	}

	// MARK_MAXWIT_TRAINING {
	part_num = part_tab_read(flash, attr_tab, MAX_FLASH_PARTS);
	if (part_num < 0)
	{
		printf("fail to read part table! (ret = %d)\n", part_num);
		ret = part_num;
		goto L2;
	}

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

		case PT_BL_GCONF: // must be defined and the the last GB partition
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
	// } MARK_MAXWIT_TRAINING

	BUG_ON(root_idx >= part_num);

	if (linux_conf->boot_mode & BM_NFS)
	{
		str += sprintf(str, "root=/dev/nfs rw nfsroot=%s:%s",
					server_ip, linux_conf->nfs_path);
	}
	else if (linux_conf->boot_mode & BM_FLASHDISK)
	{
		int root_type = attr_tab[root_idx].part_type;

		switch (root_type)
		{
		case PT_FS_CRAMFS:
		case PT_FS_JFFS2:
		case PT_FS_YAFFS:
		case PT_FS_YAFFS2:
			str += sprintf(str, "root=/dev/mtdblock%d rw rootfstype=%s", // fixme
						mtd_root, part_type2str(root_type));
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

// fixme!!
int main(int argc, char *argv[])
{
	int  ret = 0, rebuild = 0;
	u32  dev_num;
	char cmd_line[DEFAULT_KCMDLINE_LEN];
	BOOL show_args = FALSE;
	u8 *kernel_image, *ramdisk_image;
	u32 image_size;
	struct linux_config *linux_conf;
	struct net_config   *net_conf;
	struct tag *arm_tag;
	LINUX_KERNEL_ENTRY exec_linux = NULL;
	char *arg, *p;
	signed char opt;

	linux_conf = sysconf_get_linux_param();
	net_conf = sysconf_get_net_info();

	while ((opt = getopt(argc, argv, "t::r::f::n::m:c:vl:h", &arg)) != -1)
	{
		switch (opt)
		{
		case 't':
			if (arg == NULL)
			{
				linux_conf->kernel_image[0] = '\0';
			}
			else
			{
				strcpy(linux_conf->kernel_image, arg);
			}

			break;

		case 'r':
			linux_conf->boot_mode = BM_RAMDISK;

			if (arg == NULL)
			{
				linux_conf->ramdisk_image[0] = '\0';
			}
			else
			{
				strcpy(linux_conf->ramdisk_image, arg);
			}

			break;

		case 'f':
			linux_conf->boot_mode = BM_FLASHDISK;

			if (arg == NULL)
				break;

			if (string2value(arg, &dev_num) < 0)
			{
				printf("Invalid partition number (%s)!\n", arg);
			}
			else
			{
				// fixme: if num > nParts
				linux_conf->root_dev = dev_num;
			}

			break;

		case 'n':
			linux_conf->boot_mode = BM_NFS;

			if (arg == NULL)
				break;

			p = arg;

			while (*p && *p != ':' && *p != '/') p++;

			switch (*p)
			{
			case ':':
				*p = '\0';
				strcpy(linux_conf->nfs_path, p + 1);

			case '\0':
				if (str_to_ip((u8 *)&net_conf->server_ip, arg) < 0)
					printf("wrong ip format! (%s)\n", arg);

				break;

			default:
				strcpy(linux_conf->nfs_path, arg);
				break;
			}

			break;

		case 'm':
			string2value(arg, (u32 *)&linux_conf->mach_id);

			break;

		case 'c':
			strcpy(linux_conf->console_device, arg);

			break;

		case 'v':
			show_args = TRUE;
			break;

		case 'l':
			if (!strcmp(arg, "auto"))
			{
				rebuild = 1;
			}
			else
			{
				strncpy(linux_conf->cmd_line, arg, DEFAULT_KCMDLINE_LEN);
			}

			break;

		default:
			ret = -EINVAL;
			printf("Invalid option: \"%c: %s\"!\n", opt, arg);
		case 'h':
			boot_usage();
			return ret;
		}
	}

	if (rebuild)
		build_command_line(linux_conf->cmd_line, DEFAULT_KCMDLINE_LEN);

	strncpy(cmd_line, linux_conf->cmd_line, DEFAULT_KCMDLINE_LEN);

	if (argc > 2 || (2 == argc && FALSE == show_args))
	{
		sysconf_save();
	}

	BUG_ON ((linux_conf->boot_mode & ~BM_MASK) == 0);

	if (!show_args)
	{
		if (linux_conf->kernel_image[0] != '\0') // fixme for error return (i.e. permission denied)
		{
			ret = tftp_load_image(PT_OS_LINUX, linux_conf->kernel_image, &kernel_image, &image_size);
		}
		else
		{
			ret = part_load_image(PT_OS_LINUX, &kernel_image, &image_size);
		}

		if (ret < 0)
		{
			goto L1;
		}

		printf("\n");

		exec_linux = (LINUX_KERNEL_ENTRY)kernel_image;

		if (check_image_type(PT_OS_LINUX, kernel_image + UIMAGE_HEAD_SIZE))
			exec_linux = (LINUX_KERNEL_ENTRY)(kernel_image + UIMAGE_HEAD_SIZE);
		else if (!check_image_type(PT_OS_LINUX, kernel_image))
			printf("Warning: no Linux kernel image found!\n");

		DPRINT("kernel is loaded @ 0x%08x\n", exec_linux);
	}

	arm_tag = begin_setup_atag((void *)ATAG_BASE);
	arm_tag = setup_cmdline_atag(arm_tag, cmd_line);

	if (linux_conf->boot_mode & BM_RAMDISK)
	{
		if (!show_args)
		{
			if (linux_conf->ramdisk_image[0] != '\0')
			{
				ret = tftp_load_image(PT_FS_RAMDISK, linux_conf->ramdisk_image, &ramdisk_image, &image_size);
			}
			else
			{
				ret = part_load_image(PT_FS_RAMDISK, &ramdisk_image, &image_size);
			}

			if (ret < 0)
			{
				goto L1;
			}
		}

		// TODO: check the ramdisk

		arm_tag = setup_initrd_atag(arm_tag, ramdisk_image, image_size);
	}

	arm_tag = setup_mem_atag(arm_tag);

	end_setup_atag(arm_tag);

	if (show_args)
	{
		printf("\nMachine ID = %d (0x%x)\n", linux_conf->mach_id, linux_conf->mach_id);
		show_boot_args((void *)ATAG_BASE);
		return 0;
	}

	// fixme
	irq_disable();

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
