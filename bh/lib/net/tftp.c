#include <net/tftp.h>
#include <getopt.h>
#include <sysconf.h>

struct tftp_packet
{
	u16 op_code;
	union
	{
		u16 block;
		u16 error;
	};
	u8 data[0];
} __PACKED__;

static int tftp_make_rrq(u8 *buff, const char *file_name)
{
	int len;

	*(u16 *)buff = TFTP_RRQ;
	len = 2;

	strcpy((char *)buff + len, file_name);
	len += strlen(file_name);

	buff[len] = '\0';
	len += 1;

	strcpy((char *)buff + len, TFTP_MODE_OCTET);
	len += strlen(TFTP_MODE_OCTET);

	buff[len] = '\0';
	len += 1;

	return len;
}

static void tftp_send_ack(const int fd, const u16 blk, struct sockaddr_in *remote_addr)
{
	struct tftp_packet tftp_pkt;

	tftp_pkt.op_code = TFTP_ACK;
	tftp_pkt.block = htons(blk);

	sendto(fd, &tftp_pkt, TFTP_HDR_LEN, 0,
		(const struct sockaddr*)remote_addr, sizeof(*remote_addr));
}

// fixme
int tftp_download(struct tftp_opt *opt)
{
	int  ret;
	int  sockfd;
	u8   buf[TFTP_BUF_LEN];
	char server_ip[IPV4_STR_LEN], local_ip[IPV4_STR_LEN];
	u32  pkt_len, load_len;
	u16  blk_num;
	u8  *buff_ptr;
	u32  client_ip;
	struct tftp_packet *tftp_pkt;
	struct sockaddr_in *local_addr, *remote_addr;
	struct bdev_file *file;
	socklen_t addrlen;

	ndev_ioctl(NULL, NIOC_GET_IP, &client_ip);

	if (ip_to_str(local_ip, client_ip) < 0)
	{
		printf("Error: Local IP!\n");
		return -EINVAL;
	}

	if (ip_to_str(server_ip, opt->server_ip) < 0)
	{
		printf("Error: Server IP!\n");
		return -EINVAL;
	}

	tftp_pkt = (struct tftp_packet *)buf;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	if (sockfd <= 0)
	{
		printf("%s(): error @ line %d!\n", __func__, __LINE__);
		return -EIO;
	}

	local_addr = malloc(sizeof(*local_addr));
	if (local_addr == NULL)
	{
		printf("%s(): error @ line %d!\n", __func__, __LINE__);
		return -ENOMEM;
	}

	memset(local_addr, 0, sizeof(*local_addr));
	// local_addr->sin_port = port_alloc(SOCK_DGRAM);
	local_addr->sin_addr.s_addr = client_ip;
	ret = bind(sockfd, (struct sockaddr *)local_addr, sizeof(struct sockaddr));

	printf(" \"%s\": %s => %s\n", opt->file_name, server_ip, local_ip);

	pkt_len = tftp_make_rrq((u8 *)tftp_pkt, opt->file_name);

	remote_addr = malloc(sizeof(*remote_addr));
	if (remote_addr == NULL)
	{
		printf("%s(): error @ line %d!\n", __func__, __LINE__);
		return -ENOMEM;
	}

	memset(remote_addr, 0, sizeof(*remote_addr));
	remote_addr->sin_addr.s_addr = opt->server_ip; // bigendian
	remote_addr->sin_port = htons(STD_PORT_TFTP);

	sendto(sockfd, tftp_pkt, pkt_len, 0,
		(struct sockaddr *)remote_addr, sizeof(*remote_addr));

	file     = opt->file;
	buff_ptr = opt->load_addr;
	load_len = 0;
	blk_num  = 1;

	// fixme!!!
	opt->type = "jffs2";

	if (file) {
		ret = file->open(file, opt->type);
		if (ret < 0) {
			printf("fail to open \"%s\"!\n", file->bdev->dev.name);
			goto L1;
		}
	}

	do
	{
		pkt_len = recvfrom(sockfd, tftp_pkt, TFTP_BUF_LEN, 0,
						(struct sockaddr *)remote_addr, &addrlen);
		if(0 == pkt_len)
			goto L1;

		pkt_len -= TFTP_HDR_LEN;

		if (pkt_len > TFTP_PKT_LEN)  // fixme
			pkt_len = TFTP_PKT_LEN;

		switch (tftp_pkt->op_code)
		{
		case TFTP_DAT:
			if (ntohs(tftp_pkt->block) == blk_num)
			{
				tftp_send_ack(sockfd, blk_num, remote_addr);
				blk_num++;

				load_len += pkt_len;

#ifdef TFTP_VERBOSE
				if ((load_len & 0x3fff) == 0 || TFTP_PKT_LEN != pkt_len)
				{
					char tmp[32];

					val_to_hr_str(load_len, tmp);
					printf("\r %d(%s) loaded  ", load_len, tmp);
				}
#endif

				if (NULL != buff_ptr)
				{
					memcpy(buff_ptr, tftp_pkt->data, pkt_len);
					buff_ptr += pkt_len;
				}

				if (file) {
					ret = file->write(file, tftp_pkt->data, pkt_len);
					if (ret < 0)
						goto L1;
				}
			}
			else
			{
#ifdef TFTP_DEBUG
				printf("\t%s(): LOST Packet = 0x%x (0x%x).\r",
					__func__, blk_num, ntohs(tftp_pkt->block));
#endif
				tftp_send_ack(sockfd, blk_num - 1, remote_addr);
			}

			break;

		case TFTP_ERR:
			printf("\n%s(): %s (Error num = %d)\n",
				__func__, tftp_pkt->data, ntohs(tftp_pkt->error));

			ret = -EIO;
			goto L1;

		default:
			printf("\n%s(): Unsupported opcode 0x%02x! (CurBlkNum = %d)\n",
				__func__, ntohs(tftp_pkt->op_code), blk_num);

			ret = -EIO;
			goto L1;
		}
	} while (TFTP_PKT_LEN == pkt_len);

L1:
#ifdef TFTP_VERBOSE
	printf("\n");
#endif

	if (file)
		file->close(file);
	sk_close(sockfd);
	free(remote_addr);
	free(local_addr);

	return load_len;
}

