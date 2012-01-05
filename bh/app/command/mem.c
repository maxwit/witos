#include <list.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define MAX_VAL_COUNT 100

struct heap_region {
	__u32 pre_size;
	__u32 curr_size;
	struct list_node ln_mem_region;
};

#define MEM_DUMP_BASE_PRINT(base, p) do {\
								printf( "0x%08x: base base base base base base base base"\
								"  base base base base base base base base  ",\
								p, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],\
								p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);\
								} while (0)
#define MEM_DUMP_CHR_PRINT(bp) do {\
							printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",\
							bp[0], bp[1], bp[2], bp[3], bp[4], bp[5], bp[6], bp[7],\
							bp[8], bp[9], bp[10], bp[11], bp[12], bp[13], bp[14], bp[15]);\
							} while (0)

static int mem_dump(int argc, char *argv[])
{
	int i = 100, j = 0;
	int ch;
	char *p = (char *)0, bp[16];
	int flag = 0;
	char format = 'x';
	int ret = 0;
	int chr_flag = 0;

	while ((ch = getopt(argc, argv, "a:l:d:h")) != -1) {
		switch(ch) {
		case 'a':
			if ((str_to_val(optarg, (unsigned long *)&p)) < 0) {
				printf("Invaild argument\n");
				usage();
				return -EINVAL;
			}
			flag = 1;
			break;

		case 'l':
			if ((str_to_val(optarg, (unsigned long *)&i)) < 0) {
				printf("Invaild argument\n");
				usage();
				return -EINVAL;
			}
			flag = 1;
			break;

		case 'd':
			format = *optarg;
			break;

		default:
			ret = -EINVAL;
		case 'h':
			usage();
			return ret;
		}
	}

	if (flag != 1) {
		usage();
		return -EINVAL;
	}

	i = (i + 16) >> 4;

	while (i--) {
		int k;

		memcpy(bp ,p , 16);
		for (k = 0; k < 16; k++) {
			if (bp[k] < 0x20 || bp[k] > 0x7e)
				bp[k] = '.';
		}

		switch (format) {
		case 'o':
			chr_flag = 1;
		case 'O':
			MEM_DUMP_BASE_PRINT(%03o, p);
			break;

		case 'd':
			chr_flag = 1;
		case 'D':
			MEM_DUMP_BASE_PRINT(%03d, p);
			break;

		case 'x':
			chr_flag = 1;
		case 'X':
			MEM_DUMP_BASE_PRINT(%02x, p);
			break;
		}

		if (chr_flag) {
			MEM_DUMP_CHR_PRINT(bp);
		}

		j++;
		if (4 == j) {
			putchar('\n');
			j = 0;
		}

		p += 16;
	}

	return 0;
}

static int mem_write(int argc, char *argv[])
{
	int i = 0;
	int val_count = argc - 1;
	int ch;
	char *p = (char *)0;
	__u32 val[MAX_VAL_COUNT];
	int unit = 1;
	int flag = 0;
	int ret = 0;

	while ((ch = getopt(argc, argv, "a::w:h")) != -1) {
		switch (ch) {
		case 'a':
			if ((str_to_val(optarg, (unsigned long *)&p)) < 0) {
				printf("Invaild argument\n");
				return -EINVAL;
			}

			val_count -= 2;
			flag = 1;
			break;

		case 'w':
			if ((str_to_val(optarg, (unsigned long *)&unit)) < 0) {
				printf("Invaild argument\n");
				return -EINVAL;
			}
			val_count -= 2;
			break;

		default:
			ret = -EINVAL;
		case 'h':
			usage();
			return ret;
		}
	}

	if (flag != 1 || (unit != 1 && unit != 2 && unit != 4 && unit != 8)) {
		usage();
		return -EINVAL;
	}

	while (i < val_count) {
		if ((str_to_val(argv[optind + i], (unsigned long *)&val[i])) < 0) {
			printf("Invaild argument\n");
			usage();
			return -EINVAL;
		}
		i++;
	}

	//fix me
	while (--i >= 0) {
		memcpy(p + unit * i, val + i, unit);
	}
	printf("write val to addr success\n");

	return 0;
}

static int mem_set(int argc, char *argv[])
{
	int i = 1;
	int k;
	int ch;
	int val = 0;
	char *p = (char *)0;
	int flag = 0;
	int unit = 1;
	int ret = 0;
	int tmp;

	while ((ch = getopt(argc, argv, "a:v:l:w:h")) != -1) {
		switch(ch) {
		case 'a':
			if ((str_to_val(optarg, (unsigned long *)&p)) < 0) {
				printf("Invaild argument\n");
				usage();
				return -EINVAL;
			}
			flag = 1;
			break;

		case 'l':
			if ((hr_str_to_val(optarg, (unsigned long *)&i)) < 0) {
				printf("Invaild argument\n");
				usage();
				return -EINVAL;
			}
			break;

		case 'w':
			if ((str_to_val(optarg, (unsigned long *)&unit)) < 0) {
				printf("Invaild argument\n");
				usage();
				return -EINVAL;
			}
			break;

		case 'v':
			if ((str_to_val(optarg, (unsigned long *)&val)) < 0) {
				printf("Invaild argument\n");
				usage();
				return -EINVAL;
			}
			break;

		default:
			ret = -EINVAL;
		case 'h':
			usage();
			return ret;
		}
	}

	if (flag != 1) {
		usage();
		return -EINVAL;
	}

	if (unit != 1 && unit != 2 && unit != 4) {
		usage();
		return -EINVAL;
	}

	tmp = unit;
	while (tmp >>= 1) {
		i >>= 1;
	}

	k = --i;

	//fix me
	while (i >= 0) {
		memcpy(p + unit * i, &val, unit);
		i--;
	}

	printf("set addr%p to addr%p by val 0x%x success\n", p, p + k * unit, val);

	return 0;
}

static int mem_copy_move(int argc, char *argv[])
{
	int ch;
	__u32 val;
	char *src = (char *)0;
	char *dest = (char *)0;
	int flag = 0;
	int ret = 0;

	while ((ch = getopt(argc, argv, "s:d:l:")) != -1) {
		switch (ch) {
		case 's':
			if ((str_to_val(optarg, (unsigned long *)&src)) < 0) {
				printf("Invaild argument\n");

				return -EINVAL;
			}
			flag++;
			break;

		case 'd':
			if ((str_to_val(optarg, (unsigned long *)&dest)) < 0) {
				printf("Invaild argument\n");
				return -EINVAL;
			}
			flag++;
			break;

		case 'l':
			if ((hr_str_to_val(optarg, (unsigned long *)&val)) != 0) {
				printf("INvaild argument\n");
				return -EINVAL;
			}
			flag++;
			break;

		default:
			ret = -EINVAL;
		case 'h':
			usage();
			return ret;
		}
	}

	if (flag != 3) {
		usage();
		return -EINVAL;
	}

	if (strcmp(argv[0], "move") == 0) {
		memmove(dest, src, val);
		printf("mem move %d byte from addr%p to addr%p success\n", val, src, dest);
	} else if (strcmp(argv[0], "copy") == 0) {
		memcpy(dest, src, val);
		printf("mem cpy %d byte from addr%p to addr%p success\n", val, src, dest);
	}

	return 0;
}

static int mem_free(int argc, char *argv[])
{
	int sum_curr_size = 0;
	struct list_node *heap_head, *iter;
	struct heap_region *curr_region;
	int ch;
	int ret = 0;

	heap_head = get_heap_head_list();

	list_for_each(iter, heap_head) {
		curr_region = container_of(iter, struct heap_region, ln_mem_region);
		sum_curr_size += curr_region->curr_size;
	}

	while ((ch = getopt(argc, argv, "dh")) != -1) {
		switch (ch) {
		case 'd':
			printf("heap totle size: %d  used : %d unused %d\n", CONFIG_HEAP_SIZE, CONFIG_HEAP_SIZE - sum_curr_size, sum_curr_size);
			return 0;

		default:
			ret = -EINVAL;
		case 'h':
			usage();
			return ret;
		}
	}
	printf("heap totle size: 0x%x  used : 0x%x unused 0x%x\n", CONFIG_HEAP_SIZE, CONFIG_HEAP_SIZE - sum_curr_size, sum_curr_size);

	return 0;
}

int main(int argc, char *argv[])
{
	int i;
	int ret = 0;

	struct command cmd[] = {
		{
			.name = "move",
			.main = mem_copy_move
		},
		{
			.name = "copy",
			.main = mem_copy_move
		},
		{
			.name = "set",
			.main = mem_set
		},
		{
			.name = "dump",
			.main = mem_dump
		},
		{
			.name = "free",
			.main = mem_free
		},
		{
			.name = "write",
			.main = mem_write
		},
	};

	if (argc >= 2) {
		for (i = 0; i < ARRAY_ELEM_NUM(cmd); i++) {
			if (0 == strcmp(argv[1], cmd[i].name)) {
				cmd[i].main(argc - 1, argv + 1);
				return 0;
			}
		}
		ret = -EINVAL;
	}

	usage();

	return ret;
}
