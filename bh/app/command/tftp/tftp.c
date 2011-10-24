#include <getopt.h>
#include <sysconf.h>
#include <net/tftp.h>

int main(int argc, char *argv[])
{
	int ret, ch;
	struct tftp_opt dlopt;
	bool mem_only = false;

	memset(&dlopt, 0x0, sizeof(dlopt));
	net_get_server_ip(&dlopt.server_ip);

	while ((ch = getopt(argc, argv, "a:ms:f:")) != -1)
	{
		switch(ch)
		{
		case 'a':
			ret = str_to_val(optarg, (__u32 *)&dlopt.load_addr);

			if (ret < 0)
			{
				usage();
				return ret;
			}

			break;

		case 'm':
			mem_only = true;
			break;

		case 's':
			ret = str_to_ip((__u8 *)&dlopt.server_ip, optarg);
			if (ret < 0)
			{
				printf("Invalid IP: %s!\n", optarg);
				return ret;
			}
			break;

		case 'f':
			strncpy(dlopt.file_name, optarg, MAX_FILE_NAME_LEN);
			dlopt.file_name[MAX_FILE_NAME_LEN - 1] = '\0';
			break;

		case 'h':
		default:
			usage();
			return -EINVAL;
		}
	}

	if (false == mem_only)
	{
	}

	if (optind < argc)
	{
		if (optind + 1 == argc && !dlopt.file_name[0])
		{
			strncpy(dlopt.file_name, argv[optind], MAX_FILE_NAME_LEN);
			dlopt.file_name[MAX_FILE_NAME_LEN - 1] = '\0';
		}
		else
		{
			usage();
			return -EINVAL;
		}
	}

	if (!dlopt.file_name[0])
	{
	}

	ret = tftp_download(&dlopt);

	if (false == mem_only)
	{
		if (ret > 0)
		{
			conf_store();
		}
	}

	return ret;
}

