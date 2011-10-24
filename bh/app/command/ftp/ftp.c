#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <net/net.h>
#include <fs/fs.h>

#define PATH "/srv/ftp"
#define BUF_LEN 512
#define FTP_PORT 21
#define O_CREAT 100
#define O_RDWR  2

//fix me
int port_atoi(const char *str)
{
	const char *iter;
	int interger = 0;
	int temp;

	for (iter = str; *iter >= '0' && *iter <= '9'; iter++)
	{
		temp = *iter - '0';
		interger = interger * 10 + temp;
	}

	return interger;
}

static int connect_port(unsigned short port, const char *ip)
{
	int sockfd, ret;
	struct sockaddr_in server_addr, local_addr;
	__u32 svr_ip;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		return sockfd;
	}

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if (ret < 0)
	{
		printf("bind error\n");
		sk_close(sockfd);
		return ret;
	}
	str_to_ip((__u8 *)&svr_ip, ip);

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = svr_ip;

	ret = connect(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret < 0)
	{
		printf("connect error\n");
		sk_close(sockfd);

		return ret;
	}

	return sockfd;
}

static unsigned short get_data_port(int sockfd)
{
	unsigned short data_port;
	int ret;
	char buff[BUF_LEN];
	char *p;

	ret = send(sockfd, "PASV\r\n", 6, 0);
	if (ret < 0)
	{
		printf("send error\n");
		return ret;
	}

	ret = recv(sockfd, buff, BUF_LEN - 1, 0);
	if (ret < 0)
	{
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

	printf("server deliver data port: %d\n", data_port);
	return data_port;
}

static int ftp_send_cmd(int cmd_fd, const char *cmd, const char *arg)
{
	int i, j;
	char buff[BUF_LEN];
	int ret;

	for (i = 0; cmd[i]; i++)
	{
		buff[i] = cmd[i];
	}

	if (arg != NULL)
	{
		buff[i++] = ' ';
		for (j = 0; arg[j]; j++)
		{
			buff[i++] = arg[j];
		}
	}

	buff[i++] = '\r';
	buff[i++] = '\n';
	buff[i]   = '\0';

	ret = send(cmd_fd, buff, strlen(buff), 0);
	if (ret < 0)
	{
		printf("send error\n");
		return ret;
	}

	ret = recv(cmd_fd, buff, BUF_LEN - 1, 0);
	if (ret < 0)
	{
		printf("recv error\n");
		return ret;
	}
	if (!strncmp(buff, "530", 3))
	{
		ret = -1;
		printf("User or Password error\n");
	}
	else if (!strncmp(buff, "550", 3))
	{
		ret = -1;
		printf("no such file\n");
	}

	buff[ret] = '\0';

	printf("%s\n", buff);

	return ret;
}

static int ftp_download_file(int cmd_fd, const char *ip, const char *filename)
{
	int data_fd;
	int ret = 0;
	unsigned short data_port;
	char buff[BUF_LEN];
	char path[BUF_LEN];

	sprintf(path, PATH"/%s", filename);
	printf("%s\n", path);

	data_port = get_data_port(cmd_fd);
	if (data_port < 0)
	{
		printf("server deliver port error\n");
		return data_port;
	}

	data_fd = connect_port(data_port, ip);
	if (data_fd < 0)
	{
		return data_fd;
	}

	ret = ftp_send_cmd(cmd_fd, "RETR", path);
	if (ret < 0)
	{
		printf("get file error\n");
		return ret;
	}
#if 0
	fd = open(filename, O_CREAT | O_RDWR, 0666);
	if (fd < 0)
	{
		printf("open error\n");
		return fd;
	}
#endif

	while (1)
	{
		ret = recv(data_fd, buff, BUF_LEN - 1, 0);
		if (ret < 0)
		{
			printf("recv error\n");
			break;
		}
#if 0
		write(fd, buff, ret);
#endif
		buff[ret] = '\0';
		printf("%s\n", buff);

		if (ret < BUF_LEN)
		{
			break;
		}
	}
#if 0
	sk_close(fd);
#endif

	sk_close(data_fd);
	return ret;
}

int ftp_login(int cmd_fd, char *user, char *pswd) //fix me
{
	int ret;

	ret = ftp_send_cmd(cmd_fd, "USER", user);
	if (ret < 0)
	{
		return ret;
	}

	ret = ftp_send_cmd(cmd_fd, "PASS", pswd);
	if (ret < 0)
	{
		return ret;
	}

	ret = ftp_send_cmd(cmd_fd, "SYST", NULL);
	if (ret < 0)
	{
		return ret;
	}

	ret = ftp_send_cmd(cmd_fd, "TYPE", "I");
	if (ret < 0)
	{
		return ret;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int cmd_fd;
	int ret = 0;
	int ch;
	int count = 0;
	char user[BUF_LEN];
	char pswd[BUF_LEN];
	char svr_ip[BUF_LEN];
	char filename[BUF_LEN];
	char buff[BUF_LEN];

	while((ch = getopt(argc, argv, "u:p:s:f:h")) != -1)
	{
		switch(ch)
		{
		case 'u':
			strncpy(user, optarg, BUF_LEN);
			count++;
			break;

		case 'p':
			strncpy(pswd, optarg, BUF_LEN);
			count++;
			break;

		case 's':
			strncpy(svr_ip, optarg, BUF_LEN);
			count++;
			break;

		case 'f':
			strncpy(filename, optarg, BUF_LEN);
			count++;
			break;

		case 'h':
		default:
			printf("Usage: -u[...] -p[...] -s[...] -f[...]\n"
				   "-u username\n"
				   "-p password\n"
				   "-s server ip\n"
				   "-f file name\n"
		                "-h help\n       ");
			return -EINVAL;

		}
	}

	if (count != 4)
	{
	      printf("Usage: -u[...] -p[...] -s[...] -f[...]\n"
			  "-u username\n"
			  "-p password\n"
			  "-s server ip\n"
			  "-f file name\n"
			  "-h help\n       ");
		return -EINVAL;
	}
	//control connection
	cmd_fd = connect_port(FTP_PORT, svr_ip);
	if (cmd_fd < 0)
	{
		return cmd_fd;
	}

	ret = recv(cmd_fd, buff, BUF_LEN, 0);//need to wait for reply, or causes problem;
	if (ret < 0)
	{
		printf("recv error\n");
		goto L1;
	}

	//login
	ret = ftp_login(cmd_fd, user, pswd);
	if (ret < 0)
	{
		goto L1;
	}

	//download
	ret = ftp_download_file(cmd_fd, svr_ip, filename);
	if (ret < 0)
	{
		printf("download failed\n");
	}
L1:
	sk_close(cmd_fd);
	return ret;
}
