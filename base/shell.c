#include <g-bios.h>
#include <getopt.h>
#include <flash/flash.h>
#include <flash/part.h>
#include <uart/uart.h>
#include <net/net.h>


#define APP_HIST_DEPTH			    16  // should be 2**n bytes aligned.
#define APP_ADJUST_INDEX(index)    ((index) & (APP_HIST_DEPTH - 1))

#if APP_HIST_DEPTH & (APP_HIST_DEPTH - 1) != 0
#error "APP_HIST_DEPTH must power of 2!"
#endif

static int minus_position(char *buf);

struct command_stack
{
	char *cmd_stack[APP_HIST_DEPTH];
	int	  cmd_hist;
};

extern struct cmd_info flash_cmd_info[];

// fixme: DO NOT use pointer here
static struct command_stack *g_cmd_stack = NULL;

extern const struct gapp g_app_begin[], g_app_end[];


static char shell_getchar(void)
{
	char ch;

	while (1)
	{
		int ret;

		ret = uart_read(CONFIG_DBGU_ID, (u8 *)&ch, 1, WAIT_ASYNC);
		if (ret > 0)
			break;

		netif_rx_poll();
	}

	return ch;
}


// fixme
static void __INLINE__ cmd_backspace(void)
{
	printf("\033[D\033[1P");
}


static void show_prompt(void)
{
	int d;
	struct partition *part;


	part = part_open(PART_CURR, OP_RDONLY);
	if (NULL != part)
	{
		d = part_get_index(part);
		part_close(part);
	}
	else
	{
		d = part_get_home();
		printf("set to %d\n", d);
	}

	printf("g-bios: %d# ", d);
}

static void insert_one_key(char input_c, char *buf, int *cur_pos, int *cur_max)
{
	int i;

	for (i = *cur_max + 1; i > *cur_pos; --i)
		buf[i] = buf[i - 1];
	buf[i] = input_c;
	++(*cur_pos);
	++(*cur_max);

	putchar(input_c);
}


static void backspace_one_key(char *buf, int *cur_pos, int *cur_max)
{
	int i;

	for (i = *cur_pos; i < *cur_max; ++i)
		buf[i - 1] = buf[i];
	buf[*cur_max] = '\0';
	--(*cur_max);
	--(*cur_pos);

	cmd_backspace();
}


static void delete_one_key(char *buf, int *cur_pos, int *cur_max)
{
	int i;

	if (*cur_pos == *cur_max)
		return;

	for (i = *cur_pos; i < *cur_max - 1; ++i)
		buf[i] = buf[i + 1];
	buf[*cur_max] = '\0';

	--(*cur_max);

	printf("\033[1P");
}


static void show_input_buff(char *buf, const int cur_pos, const int cur_max)
{
	int i;

	for (i = 0; i < cur_max; ++i)
		putchar(buf[i]);
	for (i = cur_max; i > cur_pos; --i)
		printf("\033[D");
}


static int cmd_match(char *buf, int *cur_pos, int *cur_max)
{
	int	i = 0, j = 0, k;
	char (*pszResult)[MAX_ARG_LEN];
	char ch;
	const struct gapp *app;
	u32 nLen;
	BOOL bFlag;

	nLen = g_app_end - g_app_begin;
	pszResult = malloc(MAX_ARG_LEN * nLen);
	if (NULL == pszResult)
	{
		DPRINT("ERROR: fail to malloc, %s,%d", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}

	for (app = g_app_begin; app < g_app_end; app++)
	{
		if (!*cur_pos || strncmp(app->name, buf, *cur_pos) == 0)
			strcpy(pszResult[j++], app->name);
	}

	switch (j)
	{
	case 0:
		break;

	case 1:
		i = strlen(pszResult[0]);

		for (; *cur_pos < i; )
		{
			insert_one_key(pszResult[0][*cur_pos], buf, cur_pos, cur_max);
		}
		if (*cur_pos == *cur_max)
		{
			insert_one_key(' ', buf, cur_pos, cur_max);
		}

		break;

	default:
		for (i = *cur_pos; (ch = pszResult[0][i]); *cur_pos = ++i)
		{
			bFlag = FALSE;
			for (k = 1; k < j; k++)
				if (ch != pszResult[k][i])
					bFlag = TRUE;

			if (bFlag) break;

			insert_one_key(ch, buf, cur_pos, cur_max);
		}

		putchar('\n');
		for (i = 1; i < j + 1; i++)
		{
			printf("%-20s", pszResult[i - 1]);
			if (0 == (i & 0x3))
				putchar('\n');
		}
		if (0 != (j & 0x3))
			putchar('\n');

		show_prompt();
		show_input_buff(buf, *cur_pos, *cur_max);

		break;
	}

	free(pszResult);

	return 0;
}


static void print_input(char input_c, char *buf, int *cur_pos, int *cur_max)
{
	if (*cur_pos < MAX_ARG_LEN - 1)
	{
		insert_one_key(input_c, buf, cur_pos, cur_max);
	}
	else
	{
		putchar('\n');
		printf("error: command too long\nthe command must be less than %d letter", MAX_ARG_LEN);
		putchar('\n');
		show_prompt();
		printf(buf);
	}
}


static int cmd_up_key(char *buf, int *cur_pos, int *pindex, int *cur_max)
{
	int i;
	int index = *pindex;
	int lpos = *cur_pos;

	//save current buffer, but history index does not increase
	if (index == g_cmd_stack->cmd_hist)
	{
		buf[lpos] = '\0';

		if (NULL == g_cmd_stack->cmd_stack[g_cmd_stack->cmd_hist])
		{
			if (NULL == (g_cmd_stack->cmd_stack[g_cmd_stack->cmd_hist] = (char *)malloc(MAX_ARG_LEN)))
			{
				printf("ERROR: fail to malloc!\n");
				return -1;
			}
		}
		strcpy(g_cmd_stack->cmd_stack[g_cmd_stack->cmd_hist], buf);
	}

	index = APP_ADJUST_INDEX(index - 1);

	if (index != g_cmd_stack->cmd_hist && NULL != g_cmd_stack->cmd_stack[index])
	{
		// erase the command on screen
		for (i = 0; i < lpos; i++)
		{
			buf[i] = '\0'; //erase input buffer
			cmd_backspace();
		}

		// show history command
		printf("%s", g_cmd_stack->cmd_stack[index]);

		//update buffer & index
		strcpy(buf, g_cmd_stack->cmd_stack[index]);
		*pindex = index;
		*cur_pos = strlen(g_cmd_stack->cmd_stack[index]);
		*cur_max = *cur_pos;

		printf("\033[0K");

		return 0;
	}

	return -1;
}


static int cmd_down_key(char *buf, int *cur_pos, int *pindex, int *cur_max)
{
	int i;
	int index = *pindex;
	int lpos = *cur_pos;

	if (index == g_cmd_stack->cmd_hist)
		return 0;

	index = APP_ADJUST_INDEX(index + 1);

	// erase the command on screen
	for (i = 0; i < lpos; i++)
	{
		buf[i] = '\0'; //erase input buffer
		cmd_backspace();
	}

	// show history command
	printf("%s", g_cmd_stack->cmd_stack[index]);

	//update buffer & index
	strcpy(buf, g_cmd_stack->cmd_stack[index]);
	*pindex = index;
	*cur_pos = strlen(g_cmd_stack->cmd_stack[index]);
	*cur_max = *cur_pos;

	printf("\033[0K");

	return 0;
}


static int cmd_right_key(char *buf, int *cur_pos, int *cur_max)
{
	if (*cur_pos < *cur_max)
	{
		++(*cur_pos);
		printf("\033[C");
	}

	return 0;
}


static int cmd_left_key(char *buf, int *cur_pos, int *cur_max)
{
	if (*cur_pos > 0)
	{
		--(*cur_pos);
		printf("\033[D");
	}

	return 0;
}


static int cmd_update_history(const char *buf)
{
	if (NULL == g_cmd_stack->cmd_stack[g_cmd_stack->cmd_hist])
	{
		if (NULL == (g_cmd_stack->cmd_stack[g_cmd_stack->cmd_hist] = (char *)malloc(MAX_ARG_LEN)))
		{
			printf("ERROR: fail to malloc!\n");
			return -1;
		}
	}

	strcpy(g_cmd_stack->cmd_stack[g_cmd_stack->cmd_hist], buf);

	g_cmd_stack->cmd_hist = APP_ADJUST_INDEX(++g_cmd_stack->cmd_hist);

	return 0;
}


static char* get_minus_master(char *buf, int *cur_pos, int *sub_offz)
{
	int offset , len, head, tail, tmp;
	char *iter;
	char *temp;

	offset = minus_position(buf);
	if(offset < 0)
		return NULL;
		offset--;

	while(*(buf+offset) == ' ' )
	{
		offset--;
	}
	tail = offset;
	while(*(buf+offset) != ' ')
	{
		offset--;
	}
	offset++;
	head =  offset;
	len = tail - head + 1;
	iter = malloc(len + 1);
	temp = iter;
	if(iter == NULL)
	{
		DPRINT("ERROR: fail to malloc, %s,%d", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}
	tmp = head;
	while (len)
	{
		*iter = buf[tmp];
		iter++;
		tmp++;
		len--;
	}
	*iter = '\0';
	*sub_offz = head;
	return temp;
}


static char* get_flash_cmd(char *buf, int *cur_pos, int *sub_offs)
{
	char *sub_head, *sub_tail, *tmp;
	char *iter, *head;
	int len, count = 0;
	char *temp;

	sub_head = buf + 5;
	sub_head++;

	while(*sub_head == ' '  && count < 10)
	{
		sub_head++;
		count++;
	}

	if(count >= 10)
		return NULL;
    count = 0;

    sub_tail = sub_head;

    while(*sub_tail != ' ' && count < 10)
    {
		sub_tail++;
		count = 0;
    }

	len = sub_tail  - sub_head;
	iter = malloc(len + 1);
	temp = iter;
	if(iter == NULL)
	{
		DPRINT("ERROR: fail to malloc, %s,%d", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}

	tmp = sub_head;
	while (len)
	{
		*iter = *tmp;
		iter++;
		tmp++;
		len--;
	}
	*iter = '\0';
	*sub_offs = sub_head - buf;
	return temp;
}


static int arg_match(char *buf, int *cur_pos, int *cur_max)
{
	int	i = 0, j = 0, k, cmd_index = 0, tmp_len = 0, sub_offset;
	char* (*pszResult)[MAX_ARG_LEN];
	char ch;
	char *subcmd, *str_tmp, *save_p, *get_addr ;


	const struct cmd_info *app;

	BOOL bFlag;

	while(flash_cmd_info[cmd_index].cmd)
	{
		cmd_index++;
	}

	pszResult = malloc(cmd_index * MAX_ARG_LEN);
	if (NULL == pszResult)
	{
		DPRINT("ERROR: fail to malloc, %s,%d", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}

	subcmd = get_minus_master(buf, cur_pos, &sub_offset);
	app = &flash_cmd_info[0];
	str_tmp = zalloc(strlen(app->opt));
	save_p = str_tmp;
	for (app = &flash_cmd_info[0]; app < &flash_cmd_info[cmd_index]; app++)
	{
		if (strcmp(app->cmd, subcmd) == 0)
		{
			strcpy(str_tmp, app->opt);
			while(str_tmp < save_p + strlen(app->opt))
			{
				get_addr = strchr(str_tmp,':');
				if(get_addr == NULL)
				{
					strcpy(pszResult[j++], str_tmp);
					break;
				}
				*get_addr = '\0';
				strcpy(pszResult[j++], str_tmp);
				str_tmp = ++get_addr;
			}
		}
	}
	free(save_p);

	switch (j)
	{
	case 0:
		break;

	case 1:
		i = strlen(pszResult[0]);
		for (tmp_len = 0; tmp_len < i; tmp_len++)
		{
			insert_one_key(pszResult[0][tmp_len], buf, cur_pos, cur_max);
		}
		if (*cur_pos == *cur_max )
		{
			insert_one_key(' ', buf, cur_pos, cur_max);
		}

		break;

	default:
		for (i = *cur_pos - sub_offset; (ch = pszResult[0][i]); *cur_pos = ++i)
		{
			bFlag = FALSE;
			for (k = 1; k < j; k++)
				if (ch != pszResult[k][i])
					bFlag = TRUE;

			if (bFlag) break;

			insert_one_key(ch, buf, cur_pos, cur_max);
		}

		putchar('\n');
		for (i = 1; i < j + 1; i++)
		{
			printf("%-20s", pszResult[i - 1]);
			if (0 == (i & 0x3))
				putchar('\n');
		}
		if (0 != (j & 0x3))
			putchar('\n');

		show_prompt();
		show_input_buff(buf, *cur_pos, *cur_max);

		break;
	}

	free(pszResult);
	free(subcmd);

	return 0;
}


static int subcmd_match(char *buf, int *cur_pos, int *cur_max)
{
	int	i = 0, j = 0, k, cmd_index =0;
	char (*pszResult)[MAX_ARG_LEN];
	char ch;
	char *subcmd;
	int sub_offset;

	const struct cmd_info *app;
	BOOL bFlag;

	while(flash_cmd_info[cmd_index].cmd)
	{
		cmd_index++;
	}
	pszResult = zalloc(cmd_index * MAX_ARG_LEN);
	if (NULL == pszResult)
	{
		DPRINT("ERROR: fail to zalloc, %s,%d", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}

	subcmd = get_flash_cmd(buf, cur_pos, &sub_offset);
	for (app = &flash_cmd_info[0]; app < &flash_cmd_info[cmd_index]; app++)
	{
		if (!(*cur_pos - sub_offset) || strncmp(app->cmd, subcmd, (*cur_pos - sub_offset)) == 0)
			strcpy(pszResult[j++], app->cmd);
	}

	switch (j)
	{
	case 0:
		break;

	case 1:
		i = strlen(pszResult[0]);

		for (; (*cur_pos - sub_offset) < i; )
		{
			insert_one_key(pszResult[0][*cur_pos - sub_offset], buf, cur_pos, cur_max);
		}
		if (*cur_pos == *cur_max )
		{
			insert_one_key(' ', buf, cur_pos, cur_max);
		}

		break;

	default:
		for (i = *cur_pos - sub_offset; (ch = pszResult[0][i]); i++)
		{
			bFlag = FALSE;
			for (k = 1; k < j; k++)
				if (ch != pszResult[k][i])
					bFlag = TRUE;

			if (bFlag)
				break;

			insert_one_key(ch, buf, cur_pos, cur_max);
		}

		putchar('\n');
		for (i = 1; i < j + 1; i++)
		{
			printf("%-20s", pszResult[i - 1]);
			if (0 == (i & 0x3))
				putchar('\n');
		}
		if (0 != (j & 0x3))
			putchar('\n');

		show_prompt();
		show_input_buff(buf, *cur_pos, *cur_max);

		break;
	}

	free(pszResult);
	free(subcmd);

	return 0;
}


static int minus_position(char *buf)
{
	return strchr(buf, '-') - buf;
}


int cmd_read(char buf[])
{
	char input_c;
	int cur_pos = 0;
	int cur_max = 0;
	int hst_pos = g_cmd_stack->cmd_hist;
	int esc_sequence = 0;
	int spec_key = 0;
	int ret;

	memset(buf, 0, MAX_ARG_LEN);

	while (0 == cur_max)
	{
		input_c = 0;

		printf("\033[4h");

		show_prompt();
		printf("\033[4h");
		while (input_c != '\r' && input_c != '\n')
		{
			input_c = shell_getchar();
			//fixme: support F1 F2 F3 ...
			if (input_c == '\033')
			{
				spec_key = 1;
				input_c = shell_getchar();
				if (input_c == '[')
				{
					esc_sequence = 1;
					input_c = shell_getchar();
				}
			}
			else
			{
				spec_key = 0;
				esc_sequence = 0;
			}

			switch (input_c)
			{
			case '\t':
				if (strncmp("flash", buf, 5) == 0)
				{
					if (minus_position(buf) > 0)
					{
						if (cur_pos < minus_position(buf))
						{
							subcmd_match(buf, &cur_pos, &cur_max);
						}
						else
						{
							arg_match(buf, &cur_pos, &cur_max);
						}
					}
					else
					{
						subcmd_match(buf, &cur_pos, &cur_max);
					}
				}else{

					cmd_match(buf, &cur_pos, &cur_max);
				}
				break;

			case '\r':
			case '\n':
				putchar('\n');
				break;

			case 0x03: // ctrl + c
				cur_pos = 0;
				cur_max = 0;
				input_c = '\n';
				putchar(input_c);
				break;

			case 0x08:
			case 0x7f:
				if (cur_pos > 0)
				{
					backspace_one_key(buf, &cur_pos, &cur_max);
				}

				break;

			case 'A':
			case 'B':// no filter: escape +' [' + 'A'
				if (1 == esc_sequence)
				{
					if ('A' == input_c)
						ret = cmd_up_key(buf, &cur_pos, &hst_pos, &cur_max);
					else
						ret = cmd_down_key(buf, &cur_pos, &hst_pos, &cur_max);
				}
				else // not a up/down key, treat it as a normal input
				{
					print_input(input_c, buf, &cur_pos, &cur_max);
				}

				break;

			case 'C':
			case 'D':
				if (1 == esc_sequence)
				{
					if ('C' == input_c)
						ret = cmd_right_key(buf, &cur_pos, &cur_max);
					else
						ret = cmd_left_key(buf, &cur_pos, &cur_max);
					break;
				}
				else // not a left/right key, treat it as a normal input
				{
					print_input(input_c, buf, &cur_pos, &cur_max);
				}

				break;

			case 'O':
				if (1 == spec_key)
				{
					input_c = shell_getchar();

					if ('H' == input_c)
					{
						if (cur_pos != 0)
							printf("\033[%dD", cur_pos);
						cur_pos = 0;
					}
					if ('F' == input_c)
					{
						if (cur_pos != cur_max)
							printf("\033[%dC", cur_max - cur_pos);
						cur_pos = cur_max;
					}
				}
				else // not a Home/End key, treat it as a normal input
				{
					print_input(input_c, buf, &cur_pos, &cur_max);
				}

				break;

			case '3':
				if (1 == spec_key)
				{
					input_c = shell_getchar();

					if ('~' == input_c)
					{
						delete_one_key(buf, &cur_pos, &cur_max);
					}
				}
				else
				{
					print_input(input_c, buf, &cur_pos, &cur_max);
				}

				break;

			default:
				if (input_c >= 0x20 && input_c <= 0x7f) //filter the character
					print_input(input_c, buf, &cur_pos, &cur_max);

				break;
			}
		}
	}

	buf[cur_max] = '\0';

	cmd_update_history(buf);

	printf("\033[4h");
	printf("\033[4l");

	return 0;
}


int command_translate(const char *command_line, char *argv[])
{
	int argc = 0;
	int i = 0;
	static char cmd_line[MAX_ARG_LEN];

#if 0
	strcpy(cmd_line, command_line);

	while (1)
	{
		if (command_line[i] == '\0')
		{
			break;
		}
		else if (command_line[i] != ' ' && command_line[j] == ' ')
		{
			if (argc >= MAX_ARGC)
			{
				return -EOVERFLOW;
			}

			j = i;
			argv[argc] = cmd_line + j;
			argc++;
		}
		else if (command_line[i] == ' ' && command_line[j] != ' ')
		{
			j = i;
			cmd_line[j] = '\0';
		}

		i++;
	}

	return argc;

#else
	while (1)
	{
		if (command_line[i] != ' ')
		{
			argv[argc] = cmd_line + i;
			argc++;

			while (command_line[i] != ' ')
			{
				cmd_line[i] = command_line[i];

				if (command_line[i] == '\0')
				{
					return argc;
				}
				i++;
			}

			cmd_line[i] = '\0';
		}

		i++;

		if (command_line[i] == '\0')
		{
			return argc;
		}
	}
#endif

	return 0;
}


int cmd_exec(const char *command_line)
{
	const struct gapp *app;
	int ret;
	int argc;
	char *argv[MAX_ARGC];

	argc = command_translate(command_line, argv);

	for (app = g_app_begin; app < g_app_end; app++)
	{
		if (!strncmp(app->name, argv[0], MAX_ARG_LEN))
		{
			getopt_init();

			ret = app->main(argc, argv);
#if 0
			if (ret < 0)
			{
				printf("fail to exec %s! (exit %d)\n", argv[0], ret);
			}
#endif

			putchar('\n');

			return ret;
		}
	}

	printf("  command \"%s\" not found!\n"
		   "  Please use \"help\" to get g-bios command list.\n", argv[0]);

	return -ENOEXEC;
}


// fixme: to be removed
int __INIT__ init_cmd_queue(void)
{
	g_cmd_stack = (struct command_stack *)zalloc(sizeof(*g_cmd_stack));

	if (NULL == g_cmd_stack)
	{
		DPRINT("%s, %s(), line %d: No memory!\n",
				__FILE__, __FUNCTION__, __LINE__);

		return -ENOMEM;
	}

	return 0;
}


int exec_shell(void)
{
	init_cmd_queue();

	while (1)
	{
		char buf[MAX_ARG_LEN];

		cmd_read(buf);
		// if argc < 0
		cmd_exec(buf);
	}

	return 0;
}
