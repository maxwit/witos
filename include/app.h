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
	char *cmd;
	char *opt;
	u32 flags;
};

struct gapp
{
	const char *name;
	int (*main)(int argc, char *argv[]);
	// struct cmd_info *cmd;
	char (*option)[CMD_OPTION_LEN];
	int (*usr_cmd_match)(char *buf, int *cur_pos, int *cur_max);
	int (*usr_opt_match)(char *buf, int *cur_pos, int *cur_max);
};

#define INSTALL_APPLICATION(app_name, app_main, app_option, app_usr_cmd_match, app_usr_opt_match) \
	static const __USED__ __GBIOS_APP__ struct gapp __gbios_app_##app_name = { \
		.name = #app_name, \
		.main = app_main, \
		.option = app_option, \
		.usr_cmd_match = app_usr_cmd_match, \
		.usr_opt_match = app_usr_opt_match \
	}

void insert_one_key(char input_c, char *buf, int *cur_pos, int *cur_max);
void show_input_buff(char *buf, const int cur_pos, const int cur_max);
void show_prompt(void);
