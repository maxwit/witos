#pragma once

#include <types.h>
#include <init.h>

// to be removed
#define LM_NAND      'n'
#define LM_KERMIT    'k'
#define LM_YMODEM    'y'
#define LM_RAM       'r'
#define LM_MMC       'm'

#define __GBIOS_LOADER__    __attribute__((section(".gbios_loader")))

// fixme
#ifndef MAX_FILE_NAME_LEN
#define MAX_FILE_NAME_LEN   64
#endif

struct loader_opt
{
	void *load_addr;
	int  load_flash;
	int  load_size;
	const char *prompt;
#ifdef CONFIG_GTH
	char ckey[2];
	int (*main)(struct loader_opt *opt);
#else
	char file_name[MAX_FILE_NAME_LEN];
	struct bdev_file *file;
#endif
};

// DO NOT add "static" here
#ifdef CONFIG_GTH
#define REGISTER_LOADER(key, routine, str) \
	const __USED__ __GBIOS_LOADER__ struct loader_opt __gbios_loader_##key = \
		{.ckey = #key, .main = routine, .prompt = str}
#else
#define REGISTER_LOADER(key, routine, str)
#endif
