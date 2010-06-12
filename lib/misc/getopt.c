#include <stdio.h>
#include <string.h>


static char *opt_arg;
static int opt_idx = 1;
static int opt_pos = 1;
static int non_arg_start = 0;
static int non_arg_end = 0;


// fixme
#define SWAP(a,b) \
		do { \
			typeof(a) __temp; \
			__temp = (a); \
			(a) = (b); \
			(b) = __temp; \
		} while(0)


void getopt_init(void)
{
	opt_idx = 1;
	opt_pos = 1;
	non_arg_start = 0;
	non_arg_end = 0;
}

int getopt_idx()
{
	return opt_idx;
}

char *getopt_arg()
{
	return opt_arg;
}


static void adjust_argv(char *argv[])
{
	if (non_arg_start > 0)
	{
		int i;
		int n = non_arg_end + 1;

		while (n < opt_idx)
		{
			i = n;
			while (i > non_arg_start)
			{
				SWAP(argv[i - 1],argv[i]);
				i--;
			}
			non_arg_start++;
			non_arg_end++;
			n++;
		}

	}
}


int getopt(int argc, char *argv[], const char *optstring, char **parg)
{
	char c, *cp;

	if (1 == opt_pos)
	{

		while (opt_idx  < argc && argv[opt_idx][0] != '-')
		{
			non_arg_end = opt_idx;

			if (non_arg_start == 0)
				non_arg_start = non_arg_end;

			++opt_idx;
		}


		if (opt_idx >= argc || argv[opt_idx][0] != '-' || argv[opt_idx][opt_pos] == '\0')
		{
			//fixme
			if (non_arg_start)
				opt_idx = non_arg_start;

			return -1;
		}
	}

	c = argv[opt_idx][opt_pos];

	if (':' == c || (cp = strchr(optstring, c)) == NULL)
	{
		printf("%s:  invalid option: -- \'%c\'\n", argv[0], c);

		if (argv[opt_idx][++opt_pos] == '\0')
		{
			++(opt_idx);
			opt_pos = 1;
		}
		return '?';
	}

	if (':' == *(++cp))
	{
		if (argv[opt_idx][opt_pos + 1] != '\0')
			opt_arg = &argv[(opt_idx)++][opt_pos + 1];
		else if (++(opt_idx) >= argc || argv[opt_idx][0] == '-')
		{
			if (':' == *(++cp))
			{
				opt_arg = NULL;
				opt_pos = 1;
				goto L1;
			}

			printf("%s:  option:\'-%c\' requires an argument\n",argv[0], c);
			opt_pos = 1;

			if (':' == optstring[0])
				return ':';
			return '?';
		}
		else
			opt_arg = argv[(opt_idx)++];

		opt_pos = 1;
	}
	else
	{
		if ('\0' == argv[opt_idx][++opt_pos])
		{
			opt_pos = 1;
			(opt_idx)++;
		}

		opt_arg = NULL;
	}

L1:
	adjust_argv(argv);

	*parg = opt_arg;

	return c;

}

