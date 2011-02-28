#include <g-bios.h>
#include <graphic/gpu.h>


static const struct lcd_vmode g_lcd_vm_tab[] =
{
	{
		.name   = "P043F1A4",
		.pixclk = 9200000,
		.width  = 480,
		.height = 272,
		.hbp = 4,
		.hfp = 8,
		.hpw = 41,
		.vbp = 2,
		.vfp = 4,
		.vpw = 10,
	},
	{
		.name   = "3.5\" LCD",
		.pixclk = 5600000,
		.width  = 240,
		.height = 320,
		.hbp = 13,
		.hfp = 13,
		.hpw = 31,
		.vbp = 4,
		.vfp = 4,
		.vpw = 3,
	},
	{
		.name   = "3.5\" LCD",
		.width  = 240,
		.height = 320,
		.pixclk = 5600000, // fixme!!!!
		.vbp = 4,
		.vfp = 4,
		.vpw = 2,
		.hbp = 38,
		.hfp = 21,
		.hpw = 6,
	},

#if 0
	// end of LCD video mode table
	{
		.width = 0,
	}
#endif
};

const struct lcd_vmode *get_lcd_vmode(int lcd_id)
{
	if (lcd_id >= ARRAY_ELEM_NUM(g_lcd_vm_tab))
		return NULL;

	return g_lcd_vm_tab + lcd_id;
}

