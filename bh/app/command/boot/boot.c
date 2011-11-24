#include <sysconf.h>
#include <getopt.h>
#include <image.h>
#include <net/tftp.h>
#include <uart/uart.h>
#include <fs/fs.h>
#include <mmc/mmc.h>
#include "linux.h"
#include <flash/flash.h>

#define UIMAGE_HEAD_SIZE     64

static int mmc_load_image(PART_TYPE type, const char image_name[], __u8 **buff_ptr, __u32 *buff_len)
{
	int ret;
	int i;
	__u8 *image_buf;
	int fd;
	char dev_name[MAX_FILE_NAME_LEN];

	printf("loading %s image via mmc card:\n",
		type == PT_OS_LINUX ? "linux" : "ramdisk");

	image_buf = malloc(MB(3)); //fixme!!!
	if (image_buf == NULL) {
		printf("%s(): fail to malloc buffer!\n", __func__);
		ret = -ENOMEM;
		goto L0;
	}

	for (i = 0; i < MAX_FILE_NAME_LEN; i++) {
		if (image_name[i] == ':' || image_name[i] == '\0')
			break;

		dev_name[i] = image_name[i];
	}

	if (i == MAX_FILE_NAME_LEN) {
		DPRINT("%s(): file name length error!\n", __func__);
		ret = -EINVAL;
		goto L1;
	}

	dev_name[i] = '\0';

	ret = mount("vfat", 0, dev_name, NULL);
	if (ret < 0) {
		printf("fat_mount error, ret = %d\n", ret);
		goto L1;
	}

	fd = open(image_name, 0);
	if (fd < 0) {
		printf("open error\n");
		goto L1;
	}

	ret = read(fd, image_buf, MB(3));
	if (ret < 0) {
		printf("fail to read file %s\n", image_name);
		ret = -EINVAL;
		goto L2;
	}

	*buff_ptr = image_buf;
	*buff_len = ret;

	printf("load image OK, image size %d\n", ret);

	close(fd);

	return 0;
L2:
	close(fd);
	// fat_umount(struct part_attr * part,const char * path,const char * type,unsigned long flags)
L1:
	free(image_buf);
L0:
	return ret;
}

static int flash_load_image(PART_TYPE type, __u8 **buff_ptr, __u32 *buff_len)
{
	int  ret;
	char image_name[MAX_FILE_NAME_LEN];
	__u32  image_size;
	struct flash_chip  *flash;
	struct block_device *bdev;
	__u8 *buff;

	printf("Loading %s image from flash ... ",
		type == PT_OS_LINUX ? "kernel" : "ramdisk");

	// fixme
	flash = flash_open("mtdblock0p4");
	if (flash == NULL)
		return -ENODEV;

	bdev = &flash->bdev;

	strncpy(image_name, bdev->file->name, MAX_FILE_NAME_LEN);
	image_size = bdev->file->size;

	// to be optimized
	if (image_size <= 0 || image_size >= bdev->bdev_size) {
		printf("%s(): image seems invalid, still try to load.\n"
			"current image size = %d(0x%x), adjusted to partition size %d(0x%x)!\n",
			__func__, image_size, image_size, bdev->bdev_size, bdev->bdev_size);

		image_size = bdev->bdev_size;
	}

	buff = malloc(image_size);
	if (!buff) {
		ret = -ENOMEM;
		goto L1;
	}

	ret = flash_read(flash, buff, 0, image_size);
	if (ret < 0) {
		ret = -EIO;
		goto L1;
	}

	*buff_ptr = buff;
	*buff_len = image_size;

L1:
	flash_close(flash);

	return ret;
}

static int tftp_load_image(PART_TYPE type, char image_name[], __u8 **buff_ptr, __u32 *buff_len)
{
	int ret;
	struct tftp_opt dlopt;

	printf("Downloading %s image via tftp:\n",
		type == PT_OS_LINUX ? "linux" : "ramdisk");

	memset(&dlopt, 0x0, sizeof(dlopt));

	net_get_server_ip(&dlopt.server_ip);
	strcpy(dlopt.file_name, image_name);
	dlopt.load_addr = malloc(MB(3)); //fixme!!!
	if (!dlopt.load_addr) {
		printf("%s(): fail to malloc buffer!\n", __func__);
		return -ENOMEM;
	}

	ret = tftp_download(&dlopt);
	if (ret < 0) {
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
#if 0
	int ret = 0;
	char *str = cmd_line;
	struct net_config   *net_conf;
	struct linux_config *linux_conf;
	struct flash_chip       *flash;
	char local_ip[IPV4_STR_LEN], server_ip[IPV4_STR_LEN], net_mask[IPV4_STR_LEN];
	const char *mtd_dev;
	char part_list[512], *part_str = part_list;
	int part_num;
	int mtd_root = 0, root_idx;
	struct part_attr attr_tab[MAX_FLASH_PARTS];
	__u32 part_idx;
#ifdef CONFIG_MERGE_GB_PARTS
	__u32 gb_base, gb_size;
#endif
	__u32 client_ip, client_mask; // fixme

	memset(cmd_line, 0, max_len);

	net_conf = sysconf_get_net_info();
	linux_conf = sysconf_get_linux_param();

	ndev_ioctl(NULL, NIOC_GET_IP, &client_ip);
	ndev_ioctl(NULL, NIOC_GET_MASK, &client_mask);

	ip_to_str(local_ip, client_ip);
	ip_to_str(server_ip, server_ip);
	ip_to_str(net_mask, client_mask);

	root_idx = root_dev;

	flash = flash_open(BOOT_FLASH_ID);
	if (NULL == flash) {
		ret = -ENODEV;
		printf("fail to open flash %d\n", BOOT_FLASH_ID);
		goto L1;
	}

	mtd_dev = flash_get_mtd_name(flash);
	if (mtd_dev == NULL) {
		printf("fail to get mtd name!\n");
		goto L2;
	}

	// MARK_MAXWIT_TRAINING {
	part_num = part_tab_read(flash, attr_tab, MAX_FLASH_PARTS);
	if (part_num < 0) {
		printf("fail to read part table! (ret = %d)\n", part_num);
		ret = part_num;
		goto L2;
	}

	part_str += sprintf(part_str, "mtdparts=%s:", mtd_dev);

	for (part_idx = 0; part_idx < part_num; part_idx++) {
		struct part_attr *attr = attr_tab + part_idx;

		switch (attr->part_type) {
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
				gb_base = attr->part_base;
			gb_size += attr->part_size;
			break;

		case PT_BL_GCONF: // must be defined and the the last GB partition
			if (gb_base > attr->part_base)
				gb_base = attr->part_base;
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

	assert(root_idx < part_num);

	if (boot_mode & BM_NFS) {
		str += sprintf(str, "root=/dev/nfs rw nfsroot=%s:%s",
					server_ip, nfs_path);
	} else if (boot_mode & BM_FLASHDISK) {
		int root_type = attr_tab[root_idx].part_type;

		switch (root_type) {
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

#ifdef CONFIG_DHCP
	str += sprintf(str, " ip=dhcp");
#else
	str += sprintf(str, " ip=%s:%s:%s:%s:maxwit.googlecode.com:eth0:off",
				local_ip, server_ip, server_ip, net_mask);
#endif

	str += sprintf(str, " console=%s", console_device);

L2:
	flash_close(flash);
L1:
	return ret;

#endif
	return 0;
}

static int show_boot_args(void *tag_base)
{
	int i = 0;
	struct tag *arm_tag = tag_base;

	while (ATAG_NONE != arm_tag->hdr.tag) {
		printf("[ATAG %d] ", i);

		switch (arm_tag->hdr.tag) {
		case ATAG_CORE:
			printf("ATAG Begin\n");
			break;

		case ATAG_CMDLINE:
			printf("Kernel Command Line\n%s\n", arm_tag->u.cmdline.cmdline);
			// printf("Kernel command line\n");
			break;

		case ATAG_MEM:
			printf("Memory\n[0x%08x - 0x%08x]\n",
				arm_tag->u.mem.start, arm_tag->u.mem.start + arm_tag->u.mem.size);
			break;

		case ATAG_INITRD2:
			printf("Initrd: [0x%08x - 0x%08x]\n",
				arm_tag->u.initrd.start, arm_tag->u.initrd.start + arm_tag->u.initrd.size);
			break;

		default:
			printf("Invalid ATAG 0x%08x @ 0x%08x!\n", arm_tag->hdr.tag, arm_tag);
			return -EINVAL;
		}

		arm_tag = tag_next(arm_tag);
		i++;
	}

	return 0;
}

#define IMAGE_NAME_LEN 32
#define NFS_PATH_LEN   128
// fixme!!
int main(int argc, char *argv[])
{
	int ret = 0, auto_gen = true;
	__u32  dev_num;
	bool show_args = false;
	__u8 *kernel_image_addr = NULL, *ramdisk_image_addr = NULL;
	__u32 image_size = 0;
	struct tag *arm_tag;
	LINUX_KERNEL_ENTRY exec_linux = NULL;
	char *p;
	int opt;
	// linux
	int boot_mode = BM_FLASHDISK;
	char kernel_image[IMAGE_NAME_LEN];
	char ramdisk_image[IMAGE_NAME_LEN];
	// __u32 root_dev;
	char nfs_path[NFS_PATH_LEN];
	__u32 mach_id;
	char console_device[CONSOLE_DEV_NAME_LEN];
	char cmd_line[DEFAULT_KCMDLINE_LEN];
	// net
	__u32 server_ip;

	while ((opt = getopt(argc, argv, "t::s:r::f::n::m:c:vl:h")) != -1) {
		switch (opt) {
		case 't':
			if (optarg == NULL) {
				boot_mode = BM_FLASHDISK;
				kernel_image[0] = '\0';
			} else {
				boot_mode = BM_TFTP;
				strcpy(kernel_image, optarg);
			}

			break;

		case 's':
			boot_mode = BM_MMC;

			if (optarg == NULL)
				break;

			strcpy(kernel_image, optarg);

			break;

		case 'r':
			boot_mode = BM_RAMDISK;

			if (optarg == NULL)
				ramdisk_image[0] = '\0';
			else
				strcpy(ramdisk_image, optarg);

			break;

		case 'f':
			boot_mode = BM_FLASHDISK;

			if (optarg == NULL)
				break;

			if (str_to_val(optarg, &dev_num) < 0)
				printf("Invalid partition number (%s)!\n", optarg);
			// else
				// fixme: if num > nParts
				// root_dev = dev_num;

			break;

		case 'n':
			boot_mode |= BM_NFS;

			if (optarg == NULL)
				break;

			p = optarg;

			while (*p && *p != ':' && *p != '/') p++;

			switch (*p) {
			case ':':
				*p = '\0';
				strcpy(nfs_path, p + 1);

			case '\0':
				if (str_to_ip((__u8 *)&server_ip, optarg) < 0)
					printf("wrong ip format! (%s)\n", optarg);

				break;

			default:
				strcpy(nfs_path, optarg);
				break;
			}

			break;

		case 'm':
			str_to_val(optarg, (__u32 *)&mach_id);
			break;

		case 'c':
			strcpy(console_device, optarg);
			break;

		case 'v':
			show_args = true;
			break;

		case 'l':
			auto_gen = false;
			strncpy(cmd_line, optarg, DEFAULT_KCMDLINE_LEN);
			break;

		default:
			ret = -EINVAL;
			printf("Invalid option: \"%c: %s\"!\n", opt, optarg);
		case 'h':
			usage();
			return ret;
		}
	}

	if (auto_gen)
		build_command_line(cmd_line, DEFAULT_KCMDLINE_LEN);

	if (argc > 2 || (2 == argc && false == show_args))
		conf_store();

	assert(boot_mode & ~BM_MASK);

	if (!show_args) {
		if (boot_mode & BM_TFTP)
			ret = tftp_load_image(PT_OS_LINUX, kernel_image, &kernel_image_addr, &image_size);
		else if (boot_mode & BM_MMC)
			ret = mmc_load_image(PT_OS_LINUX, kernel_image, &kernel_image_addr, &image_size);
		else
			ret = flash_load_image(PT_OS_LINUX, &kernel_image_addr, &image_size);

		if (ret < 0) {
			printf("fail to load linux image (ret = %d)!\n", ret);
			goto L1;
		}

		printf("\n");

		exec_linux = (LINUX_KERNEL_ENTRY)kernel_image_addr;

		if (check_image_type(PT_OS_LINUX, kernel_image_addr + UIMAGE_HEAD_SIZE))
			exec_linux = (LINUX_KERNEL_ENTRY)(kernel_image_addr + UIMAGE_HEAD_SIZE);
		else if (!check_image_type(PT_OS_LINUX, kernel_image_addr))
			printf("Warning: no Linux kernel image found!\n");

		DPRINT("kernel is loaded @ 0x%08x\n", exec_linux);
	}

	arm_tag = begin_setup_atag((void *)ATAG_BASE);

	arm_tag = setup_cmdline_atag(arm_tag, cmd_line);

	if (boot_mode & BM_RAMDISK) {
		if (!show_args) {
			if (ramdisk_image[0] != '\0')
				ret = tftp_load_image(PT_FS_RAMDISK, ramdisk_image, &ramdisk_image_addr, &image_size);
			else
				ret = flash_load_image(PT_FS_RAMDISK, &ramdisk_image_addr, &image_size);

			if (ret < 0)
				goto L1;
		}

		// TODO: check the ramdisk

		arm_tag = setup_initrd_atag(arm_tag, ramdisk_image_addr, image_size);
	}

	arm_tag = setup_mem_atag(arm_tag);

	end_setup_atag(arm_tag);

	if (show_args) {
		printf("\nMachine ID = %d (0x%x)\n", mach_id, mach_id);
		show_boot_args((void *)ATAG_BASE);
		return 0;
	}

	// fixme
	irq_disable();

	exec_linux(0, mach_id, ATAG_BASE);

L1:
	printf("\n");

	return ret;
}
