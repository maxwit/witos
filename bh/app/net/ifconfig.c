#include <g-bios.h>
#include <sysconf.h>
#include <getopt.h>

static char app_option[][CMD_OPTION_LEN] = {"l", "s", "m", ":", "?", "h", "0"};

static int atoh_ex(u8 *mac, const char *str, int len)
{
	int i;
	u8 val = 0;
	const char *tmp;

	for (i = 0, tmp = str; i < len; i++, tmp++)
	{
		val = val << 4;

		if (*tmp >= '0' && *tmp <= '9')
		{
			val |= *tmp - '0';
		}
		else if (*tmp >= 'a' && *tmp <= 'f')
		{
			val |= (*tmp - 'a') + 10;
		}
		else if(*tmp >= 'A' && *tmp <= 'F')
		{
			val |=  *tmp - 'A' + 10;
		}
		else
		{
			return -1;
		}
	}

	*mac = val;

	return i;
}


static int str_to_mac(u8 *mac, const char *str)
{
	int i;
	const char *next;



	if (NULL == str)
		return -EINVAL;

	for (i = 0, next = str; i < 6; next++)
	{
		if (':' == *next)
		{
			atoh_ex((mac + i++), str, next - str);

			str = ++next;

			if (i >= 6)
				return -EINVAL;
		}
		else if ('\0' == *next)
		{
			atoh_ex((mac + i++), str, next - str);

			str = ++next;

			break;
		}
	}

	if (i < 6)
		return -EINVAL;
	else
		return 0;

}


static int show_net_info(void *pParam)
{
	u8 local[IPV4_ADR_LEN], server[IPV4_ADR_LEN];
	u8 realmac[MAC_ADR_LEN];


	net_get_ip(NULL, (u32 *)local);

    printf("\nlocal  addr: %d.%d.%d.%d\n",
		local[0],
		local[1],
		local[2],
		local[3]
		);


	net_get_server_ip((u32 *)server);

    printf("server addr: %d.%d.%d.%d\n",
		server[0],
		server[1],
		server[2],
		server[3]
		);


	net_get_mac(NULL, realmac);

	printf("MAC addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
		realmac[0],
		realmac[1],
		realmac[2],
		realmac[3],
		realmac[4],
		realmac[5]
	);

	return 0;
}


static void show_usage(void)
{
	printf("Usage:ifconfig [-l <local_addr>] [-s <serv_addr>] [-m <mac_addr>]\n");
}


int main(int argc, char *argv[])
{
	int ret = 0;
	u8  tmp_mac[6];
	u32 local_ip, server_ip;
	int opt;
	char *arg;


	while ((opt = getopt(argc, argv, "l:s:m:h", &arg)) != -1)
	{
		switch (opt)
		{
		case 'l':
			if (str_to_ip((u8 *)&local_ip, arg))
			{
				printf("Set Local IP Error! \nUsage: ifconfig -l <IP Address>\n");
				return -EINVAL;
			}

			net_set_ip(NULL, local_ip);

			break;

		case 's':
			if (str_to_ip((u8 *)&server_ip, arg))
			{
				printf("Set Server IP Error! \nUsage: ifconfig -s <IP Address>\n");
				return -EINVAL;
			}

			net_set_server_ip(server_ip);

			break;

		case 'm':
			if (str_to_mac(tmp_mac, arg))
			{
				printf("Set Mac Error! \nUsage: ifconfig -m <Mac Address>\n");
				return -EINVAL;
			}

			net_set_mac(NULL, tmp_mac);

			break;

		case ':':
		case '?':
			printf("Try `%s -h` for more information.\n",argv[0]);
			//fixme
			return -1;

		case 'h':
		default:
			show_usage();
			//fixme
			return -1;
		}
	}

	show_net_info(NULL);

	if (argc > 1)
		ret = sysconf_save();

	//fixme
	return ret;
}


