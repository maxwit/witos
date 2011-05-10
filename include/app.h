#pragma once

#include <types.h>
#include <init.h>

#define __GBIOS_APP__       __attribute__((section(".gbios_application")))
#define MAX_ARG_LEN			512
#define MAX_ARGC			64
#define CMD_OPTION_LEN					32
#define CMD_OPTION_COUNT				64
#define CMD_MAX_LEN						128

struct cmd_info
{
	const char *name;
	int (*cmd)(int argc, char *argv[]);
};

struct gapp
{
	const char *name;
	int (*main)(int argc, char *argv[]);
};

#define INSTALL_APPLICATION(app_name, app_main) \
	static const __USED__ __GBIOS_APP__ struct gapp __gbios_app_##app_name = { \
		.name = #app_name, \
		.main = app_main, \
	}

void insert_one_key(char input_c, char *buf, int *cur_pos, int *cur_max);
void show_input_buff(char *buf, const int cur_pos, const int cur_max);
void show_prompt(void);
