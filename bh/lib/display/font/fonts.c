#include <graphic/display.h>
#include <font/font.h>
#include <list.h>

struct list_node font_list;

void add_font(struct font_descript *font)
{
	struct font_list_node *f_node;

	f_node = (struct font_list_node *)malloc(sizeof(*f_node));
	if (f_node == NULL)
	{
		return;
	}

	f_node->font = font;
	list_add_tail(&f_node->node, &font_list);
}

void show_all_fonts()
{
	struct font_descript *font;
	struct font_list_node *f_node;
	struct list_node *iter;

	list_for_each(iter, &font_list)
	{
		f_node = container_of(iter, struct font_list_node, node);
		font = f_node->font;
		printf("Support font : %s\n", font->name);
	}
}

struct font_descript *find_font(const char *name)
{
	struct font_descript *font;
	struct font_list_node *f_node;
	struct list_node *iter;

	list_for_each(iter, &font_list)
	{
		f_node = container_of(iter, struct font_list_node, node);
		font = f_node->font;
		if (strcmp(name, font->name) == 0)
		{
			return font;
		}
	}

	return NULL;
}

static int __INIT__ font_init()
{
	list_head_init(&font_list);

	return 0;
}

FONT_LIST_INIT(font_init);
