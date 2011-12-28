#include <getopt.h>
#include <net/net.h>
#include <net/tftp.h>
#include <net/socket.h>

#define TFTP_DEBUG
// fixme: to be removed
// #define FILE_READ_SUPPORT

struct tftp_packet {
	__u16 op_code;
	union {
		__u16 block;
		__u16 error;
	};
	__u8 data[0];
} __PACKED__;

static int tftp_make_req(__u8 *buff, __u16 req, const char *file_name, const char *mode)
{
	int len;

	*(__u16 *)buff = req; // TFTP_RRQ or TFTP_WRQ
	len = 2;

	strcpy((char *)buff + len, file_name);
	len += strlen(file_name);

	buff[len] = '\0';
	len += 1;

	strcpy((char *)buff + len, mode);
	len += strlen(mode);

	buff[len] = '\0';
	len += 1;

	return len;
}

static int tftp_send_ack(const int fd, const __u16 blk, struct sockaddr_in *remote_addr)
{
	int ret;
	struct tftp_packet tftp_pkt;

	tftp_pkt.op_code = TFTP_ACK;
	tftp_pkt.block = htons(blk);
	ret = sendto(fd, &tftp_pkt, TFTP_HDR_LEN, 0,
			(struct sockaddr *)remote_addr, sizeof(*remote_addr));

	return ret;
}

// fixme
int tftp_download(struct tftp_opt *opt)
{
	int ret;
	int sockfd;
	__u16 blk_num;
	__u8 *buff_ptr;
	socklen_t addrlen;
	__u8 buf[TFTP_BUF_LEN];
	size_t  pkt_len, load_len;
	struct bdev_file *file;
	struct tftp_packet *tftp_pkt = (struct tftp_packet *)buf;
	struct sockaddr_in local_addr, remote_addr;
	char server_ip[IPV4_STR_LEN];

	if (ip_to_str(server_ip, opt->server_ip) < 0) {
		printf("Error: Server IP!\n");
		return -EINVAL;
	}

	// printf(" \"%s\": %s => %s\n", opt->file_name, server_ip, local_ip);
	printf("loading file \"%s\" from %s\n", opt->file_name, server_ip);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd <= 0) {
		printf("%s(): error @ line %d!\n", __func__, __LINE__);
		return -EIO;
	}

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ret = bind(sockfd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr));
	if (ret < 0)
		goto L1;

	pkt_len = tftp_make_req((__u8 *)tftp_pkt, TFTP_RRQ, opt->file_name,
				opt->mode[0] ? opt->mode : TFTP_MODE_OCTET);

	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_addr.s_addr = opt->server_ip; // bigendian
	remote_addr.sin_port = htons(STD_PORT_TFTP);

	ret = sendto(sockfd, tftp_pkt, pkt_len, 0,
		(struct sockaddr *)&remote_addr, sizeof(remote_addr));
	if (ret < 0)
		goto L1;

	file     = opt->file;
	buff_ptr = opt->load_addr;
	load_len = 0;
	blk_num  = 1;

	if (file) {
		ret = file->open(file, 0);
		if (ret < 0) {
			printf("fail to open \"%s\"!\n", file->bdev->name);
			goto L1;
		}
	}

	do {
		pkt_len = recvfrom(sockfd, tftp_pkt, TFTP_BUF_LEN, 0,
						(struct sockaddr *)&remote_addr, &addrlen);
		if(0 == pkt_len)
			goto L2;

		pkt_len -= TFTP_HDR_LEN;

		if (pkt_len > TFTP_PKT_LEN)  // fixme
			pkt_len = TFTP_PKT_LEN;

		switch (tftp_pkt->op_code) {
		case TFTP_DAT:
			if (ntohs(tftp_pkt->block) == blk_num) {
				tftp_send_ack(sockfd, blk_num, &remote_addr);
				blk_num++;

				load_len += pkt_len;

#ifdef TFTP_VERBOSE
				if ((load_len & 0x3fff) == 0 || TFTP_PKT_LEN != pkt_len) {
					char tmp[32];

					val_to_hr_str(load_len, tmp);
					printf("\r %d(%s) loaded  ", load_len, tmp);
				}
#endif

				if (NULL != buff_ptr) {
					memcpy(buff_ptr, tftp_pkt->data, pkt_len);
					buff_ptr += pkt_len;
				}

				if (file) {
					ret = file->write(file, tftp_pkt->data, pkt_len);
					if (ret < 0)
						goto L2;
				}
			} else {
#ifdef TFTP_DEBUG
				printf("\t%s(): LOST Packet = 0x%x (0x%x).\r",
					__func__, blk_num, ntohs(tftp_pkt->block));
#endif
				tftp_send_ack(sockfd, blk_num - 1, &remote_addr);
			}

			break;

		case TFTP_ERR:
			printf("\n%s(): %s (Error num = %d)\n",
				__func__, tftp_pkt->data, ntohs(tftp_pkt->error));

			ret = -EIO;
			goto L2;

		default:
			printf("\n%s(): Unsupported opcode 0x%02x! (CurBlkNum = %d)\n",
				__func__, ntohs(tftp_pkt->op_code), blk_num);

			ret = -EIO;
			goto L2;
		}
	} while (TFTP_PKT_LEN == pkt_len);

	opt->xmit_size = load_len;
L2:
#ifdef TFTP_VERBOSE
	printf("\n");
#endif

	if (file)
		file->close(file);
L1:
	sk_close(sockfd);
	return ret;
}

int tftp_upload(struct tftp_opt *opt)
{
	int ret;
	int sockfd;
	__u16 blk_num;
	char *buff_ptr;
	socklen_t addrlen;
	__u8 buf[TFTP_BUF_LEN];
	size_t pkt_len, send_len, dat_len;
	struct tftp_packet *tftp_pkt = (struct tftp_packet *)buf;
	struct sockaddr_in local_addr, remote_addr;
	char server_ip[IPV4_STR_LEN];

	if (ip_to_str(server_ip, opt->server_ip) < 0) {
		printf("Error: Server IP!\n");
		return -EINVAL;
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd <= 0) {
		printf("%s(): error @ line %d!\n", __func__, __LINE__);
		return -EIO;
	}

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ret = bind(sockfd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr));
	if (ret < 0)
		goto L1;

	printf("putting file \"%s\" to %s\n", opt->file_name, server_ip);

	pkt_len = tftp_make_req((__u8 *)tftp_pkt, TFTP_WRQ, opt->file_name,
				opt->mode[0] ? opt->mode : TFTP_MODE_OCTET);

	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_addr.s_addr = opt->server_ip; // bigendian
	remote_addr.sin_port = htons(STD_PORT_TFTP);

	ret = sendto(sockfd, tftp_pkt, pkt_len, 0,
		(struct sockaddr *)&remote_addr, sizeof(remote_addr));
	if (ret < 0)
		goto L1;

#ifdef FILE_READ_SUPPORT
	file = opt->file;
#endif
	buff_ptr = opt->load_addr;
	send_len = 0;
	blk_num  = 0;
	int file_size = 10230; //fixme!! ?

#ifdef FILE_READ_SUPPORT
	if (file) {
		ret = file->open(file, opt->type);
		if (ret < 0) {
			printf("fail to open \"%s\"!\n", file->bdev->name);
			goto L1;
		}
	}
#endif

	do {
		pkt_len = recvfrom(sockfd, tftp_pkt, TFTP_BUF_LEN, 0,
						(struct sockaddr *)&remote_addr, &addrlen);
		if(0 == pkt_len)
			goto L2;

		switch (tftp_pkt->op_code) {
		case TFTP_ACK:
			if (ntohs(tftp_pkt->block) == blk_num) {
				dat_len = file_size > TFTP_PKT_LEN ? TFTP_PKT_LEN : file_size;
#ifdef FILE_READ_SUPPORT
				ret = file->read(file, tftp_pkt->data, dat_len);
				blk_num++;

				tftp_pkt->op_code = TFTP_DAT;
				tftp_pkt->block= htons(blk_num);

				pkt_len += TFTP_HDR_LEN;
#endif
				if (NULL != buff_ptr) {
					blk_num++;

					// make TFTP data packet
					tftp_pkt->op_code = TFTP_DAT;
					tftp_pkt->block   = htons(blk_num);
					memcpy(tftp_pkt->data, buff_ptr, dat_len);
				}

				ret = sendto(sockfd, tftp_pkt, pkt_len, 0,
					(struct sockaddr *)&remote_addr, sizeof(remote_addr));
				// if ret < 0

				send_len  += dat_len;
				file_size -= dat_len;
				buff_ptr  += dat_len;

#ifdef TFTP_VERBOSE
				if ((send_len & 0x3fff) == 0 || TFTP_PKT_LEN != pkt_len) {
					char tmp[32];

					val_to_hr_str(send_len, tmp);
					printf("\r %d(%s) sended  ", send_len, tmp);
				}
#endif
			} else {
#ifdef TFTP_DEBUG
				printf("\t%s(): LOST Packet = 0x%x (0x%x).\r",
					__func__, blk_num, ntohs(tftp_pkt->block));
#endif
				pkt_len = recvfrom(sockfd, tftp_pkt, TFTP_BUF_LEN, 0,
								(struct sockaddr *)&remote_addr, &addrlen);
			}

			break;

		case TFTP_ERR:
			printf("\n%s(): %s (Error num = %d)\n",
				__func__, tftp_pkt->data, ntohs(tftp_pkt->error));

			ret = -EIO;
			goto L2;

		default:
			printf("\n%s(): Unsupported opcode 0x%02x! (CurBlkNum = %d)\n",
				__func__, ntohs(tftp_pkt->op_code), blk_num);

			ret = -EIO;
			goto L2;
		}
	} while (file_size > 0);
	
	opt->xmit_size = send_len;
L2:
#ifdef TFTP_VERBOSE
	printf("\n");
#endif

#ifdef FILE_READ_SUPPORT
	if (file)
		file->close(file);
#endif

L1:
	sk_close(sockfd);
	return ret;
}
