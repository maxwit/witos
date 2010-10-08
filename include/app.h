#pragma once

#include <types.h>
#include <init.h>


#define __GBIOS_APP__       __attribute__((section(".gbios_application")))
#define MAX_ARG_LEN  		512
#define MAX_ARGC			64

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
	struct cmd_info *cmd;
};


#define INSTALL_APPLICATION(app_name, app_main) \
	static const __USED__ __GBIOS_APP__ struct gapp __gbios_app_##app_name =
		{.name = #app_name, .main = app_main}
