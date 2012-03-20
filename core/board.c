#include <stdio.h>
#include <string.h>
#include <board.h>
#include <errno.h>

static const struct board_id *g_active_board; // or just board_id ?

#if 0
static const struct board_id *match_id(const struct board_id *id, const char *name)
{
	while (id->name) {
		if (!strcmp(id->name, name)) // strncasecmp() instead
			return id;

		id++;
	}

	return NULL;
}
#endif

int __INIT__ board_init(void)
{
	int ret;
	char name[CONF_VAL_LEN];
	// const struct board_id *id;
	struct board_desc *board;
	extern struct board_desc g_board_start[], g_board_end[];

	ret = conf_get_attr("board.id", name);
	if (ret < 0) {
		printf("Warning: board id NOT configured!\n");
		name[0] = '\0';
		// return ret;
	}

	for (board = g_board_start; board < g_board_end; board++) {
		// id = match_id(board->id_table, name);
		if (!name[0] || !strncmp(board->name, name, sizeof(name))) {
			ret = board->init(board, board->id_table);
			if (!ret) {
				printf("board \"%s\" is active for \"%s\"\n", board->name, name);
				g_active_board = board->id_table;
				return 0;
			}

			if (name[0])
				return ret;
		}
	}

	printf("No match board found for \"%s\", available boards:\n",
		name[0] ? name : "(Unknown)");
	for (board = g_board_start; board < g_board_end; board++) {
		printf("\"%s\": id = %d\n", board->name, board->id_table[0].mach_id);
	}

	return -ENOENT;
}

const struct board_id *board_get_active()
{
	return g_active_board;
}
