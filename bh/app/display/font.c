#include <graphic/display.h>
#include <font/font.h>
#include <font/draw_screen.h>
#include <errno.h>

static void font_usage(void)
{
	printf("Usage: font [OPTION] [STR]\n"
		"Font's operater by OPTION.\n"
		"\nOPTION:\n"
		"\t-l : list all supported fonts.\n"
		"\t-t <FONT> STR: Display the STR with FONT on screen\n");
}

int main(int argc, char *argv[])
{
	struct display *disp;
	struct font_descript *font;

	font_usage();

	show_all_fonts();

	if (argc != 3)
	{
		printf("Usage: lsfont fontname string\n");
		return 0;
	}

	disp = get_system_display();
	if (disp == NULL)
	{
		return -ENODEV;
	}

	font = find_font(argv[1]);
	if (font == NULL)
	{
		printf("Not support %s font!\n", argv[1]);
		return -ENODEV;
	}



	set_screen(0xFFFF);

	draw_string(disp, 100, 100, font, 0x1F << 11, argv[2]);

	return 0;
}
