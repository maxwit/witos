#include <graphic/display.h>
#include <font/font.h>
#include <font/draw_screen.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	struct display *disp;
	struct font_descript *font;

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
