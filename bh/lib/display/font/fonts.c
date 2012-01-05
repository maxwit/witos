#include <string.h>
#include <font/font.h>

const struct font_desc *find_font(const char *name)
{
	const struct font_desc *font;
	extern const struct font_desc font_list_begin[], font_list_end[];

	for (font = font_list_begin; font < font_list_end; font++) {
		if (!strcmp(name, font->name))
			return font;
	}

	return NULL;
}
