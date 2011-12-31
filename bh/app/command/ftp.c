#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <net/net.h>
#include <net/socket.h>
#include <fs/fs.h>

#define PATH "/srv/ftp"
#define BUF_LEN 512
#define FTP_PORT 21
#define USER_PSWD_ERROR "530"
#define NO_SUCH_FILE    "550"

struct ftp_opt {
	__u16 svr_port;
	__u32 svr_ip;
	char src_file[BUF_LEN];
	char dst_file[BUF_LEN];
	char user[BUF_LEN];
	char pswd[BUF_LEN];
	char type[BUF_LEN];
	char mem[BUF_LEN];
	bool def_user_flag;
	bool mem_flag;
};

static int port_atoi(const char *str)
{
	const char *iter;
	int interger = 0;
	int temp;

	for (iter = str; *iter >= '0' && *iter <= '9'; iter++) {
		temp = *iter - '0';
		interger = interger * 10 + temp;
	}

	return interger;
}

static int connect_port(const struct ftp_opt *fopt)
{
	int sockfd, ret;
	struct sockaddr_in server_addr, local_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		return sockfd;
	}

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if (ret < 0) {
		printf("bind error\n");
		sk_close(sockfd);
		return ret;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(fopt->svr_port);
	server_addr.sin_addr.s_addr = fopt->svr_ip;

	ret = connect(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret < 0) {
		printf("connect error\n");
		sk_close(sockfd);

		return ret;
	}

	return sockfd;
}

static int get_data_port(int sockfd)
{
	int data_port;
	int ret;
	char buff[BUF_LEN];
	char *p;

	ret = send(sockfd, "PASV\r\n", 6, 0);
	if (ret < 0) {
		printf("send error\n");
		return ret;
	}

	ret = recv(sockfd, buff, BUF_LEN - 1, 0);
	if (ret < 0) {
		printf("recv error\n");
		return ret;
	}

	buff[ret] = '\0';
	printf("%s\n", buff);

	p = strrchr(buff, ',');
	data_port = port_atoi(p + 1);
	*p = '\0';

	p = strrchr(buff, ',');
	data_port += port_atoi(p + 1) << 8;

	return data_port;
}

static int ftp_send_cmd(int cmd_fd, const char *cmd, const char *arg)
{
	char buff[BUF_LEN];
	int ret;

	if (arg != NULL)
		snprintf(buff, BUF_LEN, "%s %s\r\n", cmd, arg);
	else
		sprintf(buff, "%s\r\n", cmd);

	ret = send(cmd_fd, buff, strlen(buff), 0);
	if (ret < 0) {
		printf("send error\n");
		return ret;
	}

	ret = recv(cmd_fd, buff, BUF_LEN - 1, 0);
	if (ret < 0) {
		printf("recv error\n");
		return ret;
	}

	buff[ret] = '\0';

	if (!strncmp(buff, USER_PSWD_ERROR, 3)) {
		ret = -1;
	} else if (!strncmp(buff, NO_SUCH_FILE, 3)) {
		ret = -1;
	}

	printf("%s\n", buff);

	return ret;
}

//download to flash
#if 0
static int get_file_to_flash(int data_fd)
{
	return -1;
	int ret;
	char buff[BUF_LEN];
	char cur_vol;
	struct block_device *bdev;
	struct bdev_file *file;

	cur_vol = getcwd();

	bdev = get_bdev_by_index(cur_vol);
	if (NULL== bdev) {
		printf("bdev null\n");
		return -1;
	}

	file = bdev->file;
	if (NULL == file) {
		printf("file null\n");
		return 0;
	}

	file->bdev = bdev;

	file->open(file, 0);

	while (1) {
		ret = recv(data_fd, buff, BUF_LEN, 0);
		if (ret < 0) {
			printf("recv error\n");
			break;
		}

		file->write(file, buff, ret);
		if (ret < BUF_LEN) {
			break;
		}
	}

	return 0;
}
#endif

#if 0
static int get_file_to_mem(int data_fd, const struct ftp_opt *fopt)
{
	__u32 addr;
	char *p;
	int ret;
	int i;
	char buff[BUF_LEN];

	str_to_val(fopt->mem, (unsigned long *)&addr);
	p = (char *)addr;

	while(1) {
		ret = recv(data_fd, buff, BUF_LEN - 1 , 0);
		if (ret < 0) {
			printf("recv error\n");
			break;
		}

		buff[ret] = '\0';

		//write
		for (i = 0; buff[i]; i++) {
			*p++ = buff[i];
			printf("%c", *(p - 1));
		}

		if (ret < BUF_LEN - 1)
			break;
	}

	printf("\n");

#if 0
	*p = '\0';
	p = addr;
	printf("%x\n", p);
	printf("%s\n", p);
#endif

	return 0;
}
#endif

static int ftp_download_file(int cmd_fd, struct ftp_opt *fopt)
{
	return -1;
#if 0
	int data_port;
	int data_fd;
	int ret = 0;
	char path[BUF_LEN];

	if (!fopt->def_user_flag)
		sprintf(path, PATH"/%s", fopt->src_file);
	else
		sprintf(path, "%s", fopt->src_file);

	data_port = get_data_port(cmd_fd);
	if (data_port < 0) {
		printf("server deliver port error\n");
		return data_port;
	}

	fopt->svr_port = data_port;
	data_fd = connect_port(fopt);
	if (data_fd < 0) {
		return data_fd;
	}

	ret = ftp_send_cmd(cmd_fd, "RETR", path);
	if (ret < 0) {
		return ret;
	}

	if (!fopt->mem_flag)
		get_file_to_flash(data_fd);
	else
		get_file_to_mem(data_fd, fopt);

	sk_close(data_fd);

	return ret;

#endif
}

static int ftp_upload_file(int cmd_fd,  struct ftp_opt *fopt)
{
	int data_port;
	int data_fd;
	int ret = 0;
	char buff[BUF_LEN] = "It's just for test!";
	char path[BUF_LEN];

	if (!fopt->def_user_flag)
		sprintf(path, PATH"/%s", fopt->src_file);
	else
		sprintf(path, "%s", fopt->src_file);

	data_port = get_data_port(cmd_fd);
	if (data_port < 0) {
		printf("server deliver port error\n");
		return data_port;
	}

	fopt->svr_port = data_port;

	data_fd = connect_port(fopt);
	if (data_fd < 0) {
		return data_fd;
	}

	ret = ftp_send_cmd(cmd_fd, "STOR", path);
	if (ret < 0) {
		printf("put file error\n");
		return ret;
	}

	ret = send(data_fd, buff, strlen(buff), 0);
	if (ret < 0) {
		printf("send error\n");
		return ret;
	}

	sk_close(data_fd);

	return ret;
}

static int ftp_login(int cmd_fd, const struct ftp_opt *fopt) //fix me
{
	int ret;

	ret = ftp_send_cmd(cmd_fd, "USER", fopt->user);
	if (ret < 0) {
		return ret;
	}

	ret = ftp_send_cmd(cmd_fd, "PASS", fopt->pswd);
	if (ret < 0) {
		return ret;
	}

	ret = ftp_send_cmd(cmd_fd, "SYST", NULL);
	if (ret < 0) {
		return ret;
	}

	ret = ftp_send_cmd(cmd_fd, "TYPE", fopt->type);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int get_arg(char *arg, struct ftp_opt *fopt)
{
	int i ;
	int port = 0;
	char *cp;

	//if specialize user(not anonymous)
	if (*arg != '@') {
		if ((arg = strchr(arg, ':')) != NULL)
			arg++;

		for (i = 0; *arg; arg++) {
			if (*arg == ':' || *arg == '@') {
				if (*arg == '@') {
					printf("Please specific the password\n");
					return -1;
				}
				fopt->user[i] = '\0';
				fopt->def_user_flag = false;
				break;
			}

			fopt->user[i++] = *arg;
		}

		for (i = 0, arg++; *arg ; arg++) {
			if (*arg == ':' || *arg == '@' ) {
				fopt->pswd[i] = '\0';
				break;
			}

			fopt->pswd[i++] = *arg;
		}
	}

	arg++;
	//if specialize port number
	if ((cp = strchr(arg, ':')) != NULL) {
		*cp = '\0';
		str_to_ip((__u8 *)&fopt->svr_ip, arg);
		arg = cp + 1;

		port = port_atoi(arg);
		if (port == FTP_PORT)
			fopt->svr_port = port;
		else  {
			printf("invalid port for FTP\n");
			return -1;
		}
		//if specialize filename
		if ((cp = strchr(arg, '/')) != NULL) {
			*cp = '\0';
			arg = cp + 1;
			if (*arg)
				memcpy(fopt->src_file, arg, BUF_LEN);

			*arg = '\0';
		}
	}
	//if lack of port number, and specialize filename
	else if ((cp = strchr(arg, '/'))!= NULL) {
		*cp = '\0';
		str_to_ip((__u8 *)&fopt->svr_ip, arg);
		arg = cp + 1;
		if (*arg)
			memcpy(fopt->src_file, arg, BUF_LEN);

		*arg = '\0';
	}

	//if only ip behind '@'
	if (*arg)
		str_to_ip((__u8 *)&fopt->svr_ip, arg);

	printf("user = %s, pass = %s, filename = %s \n", fopt->user,
			fopt->pswd, fopt->src_file);

	return 0;

}

int main(int argc, char *argv[])
{
	int cmd_fd;
	int ret = 0;
	int ch;
	char filename[BUF_LEN];
	char buff[BUF_LEN];
	bool get_put_flag = true;
	struct ftp_opt fopt;

	memset(filename, 0, sizeof(filename));
	memset(&fopt, 0, sizeof(fopt));

	strncpy(fopt.user, "anonymous", BUF_LEN);
	strncpy(fopt.pswd, "any", BUF_LEN);

	net_get_server_ip(&fopt.svr_ip);
	strncpy(fopt.src_file, "a", BUF_LEN);
	strncpy(fopt.type, "I", BUF_LEN);

	fopt.svr_port = FTP_PORT;
	fopt.def_user_flag = true;
	fopt.mem_flag = false;

	if (strncmp(argv[1], "put", 3) == 0) {
		get_put_flag = false;
	}

	ret = argc - 1;

	if (argc >= 2 && (!strncmp(argv[ret], "user", 4)  || *(argv[ret]) == '@') )
		ret = get_arg(argv[ret], &fopt);
		if (ret < 0)
			return ret;

	while((ch = getopt(argc, argv, "f:m:v:h")) != -1) {

		switch(ch) {
		case 'f':
			strncpy(filename, optarg, BUF_LEN);
			break;

		case 'm':
			fopt.mem_flag = true;
			strncpy(fopt.mem, optarg, BUF_LEN);
			break;

		case 'v':
			strncpy(fopt.type, optarg, BUF_LEN);
			break;

		case 'h':
		default:
			usage();
			return -EINVAL;
		}
	}

	if (get_put_flag && filename[0])
		memcpy(fopt.dst_file, filename, BUF_LEN);

	else if(!get_put_flag && filename[0]) {
		memcpy(fopt.src_file, filename, BUF_LEN);
	}

	// control connection
	cmd_fd = connect_port(&fopt);
	if (cmd_fd < 0) {
		return cmd_fd;
	}

	ret = recv(cmd_fd, buff, BUF_LEN, 0);// need to wait for reply, or causes problem;
	if (ret < 0) {
		printf("recv error\n");
		goto L1;
	}

	// login
	ret = ftp_login(cmd_fd, &fopt);
	if (ret < 0) {
		goto L1;
	}

	// download
	if (get_put_flag) {
		ret = ftp_download_file(cmd_fd, &fopt);
		if (ret < 0) {
			printf("download failed\n");
		}
	}else {//upload
		ret = ftp_upload_file(cmd_fd, &fopt);
		if (ret < 0) {
			printf("upload failed\n");
		}
	}

L1:
	sk_close(cmd_fd);

	return ret;
}
