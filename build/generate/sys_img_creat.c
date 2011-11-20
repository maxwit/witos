#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/types.h>

#define DBG_STR_LEN 128
#define LEN 512

#define GB_SYSCFG_MAGICNUM  0x5a5a5a5a

typedef __u32 u32;
typedef __u16 u16;

struct sys_config
{
	__u32 magic;
	__u32 size;
	__u32 offset;
	__u32 checksum;
};

u16 net_calc_checksum(const void *buff, u32 size)
{
	int n;
	u32	chksum;
	const u16 *p;

	chksum = 0;

	for (n = size, p = buff; n > 0; n -= 2, p++)
		chksum += *p;

	chksum = (chksum & 0xffff) + (chksum >> 16);
	chksum = (chksum & 0xffff) + (chksum >> 16);

	return chksum & 0xffff;
}

int main(int argc, char *argv[])
{
	struct sys_config sys;
	const char *txt_file, *img_file;
	char str[DBG_STR_LEN];
	int txt_fd, img_fd;
	int ret;
	struct stat file_stat;
	char buff[LEN];

	if (argc != 3)
	{
		printf("Usage: sysconf_img sysconf.txt sysconf.img\n");
		return -EINVAL;
	}

	txt_file = argv[1];
	img_file = argv[2];

	txt_fd = open(txt_file, O_RDONLY);
	if (txt_fd < 0)
	{
		sprintf(str, "open %s", txt_file);
		perror(str);
		return txt_fd;
	}

	img_fd = open(img_file, O_WRONLY | O_CREAT, 0644);
	if (img_fd < 0)
	{
		sprintf(str, "open %s", img_file);
		perror(str);
		ret = img_fd;
		goto L1;
	}

	ret = fstat(txt_fd, &file_stat);
	if (ret < 0)
	{
		printf("Get file state failed!\n");
		goto L2;
	}

	// init sysconfig head
	sys.magic  = GB_SYSCFG_MAGICNUM;
	sys.size   = file_stat.st_size;
	sys.offset = sizeof(sys);
	sys.checksum = net_calc_checksum(&sys, sizeof(sys) - 4);

	ret = write(img_fd, &sys, sizeof(sys));
	if (ret != sizeof(sys))
	{
		sprintf(str, "write %s", img_file);
		perror(str);
		goto L2;
	}

	// copy txt_file to img_file
	while ((ret = read(txt_fd, buff, LEN)) > 0)
	{
		ret = write(img_fd, buff, ret);
		if (ret < 0)
		{
			sprintf(str, "write %s", img_file);
			perror(str);
			goto L2;
		}
	}

	if (ret < 0)
	{
		sprintf(str, "read %s", txt_file);
		perror(str);
	}

L2:
	close(img_fd);
L1:
	close(txt_fd);

	return ret;
}
