#include <app.h>

static const struct option lcd_opt[] = {
	{
		.opt  = "-l [all]",
		.desc = "list video mode",
	},
	{
		.opt  = "-s <model>",
		.desc = "set LCD's video mode to the built-in model",
	},
	{
		.opt  = "-h",
		.desc = "this help",
	},
};

REGISTER_HELP_L1(lcd, "LCD video mode setup utility", lcd_opt);