#pragma once

#include <autoconf.h>

#define HEAP_SIZE   (8 << 20)

#ifndef __ASSEMBLY__
#include <types.h>
#include <errno.h>
#include <timer.h>
#include <io.h>
#endif

#include <image.h>

#ifdef CONFIG_GTH

#ifndef __ASSEMBLY__

#define GBH_LOAD_SIZE     MB(2)

int cpu_init(void);

int soc_init(void);

int uart_init(void);

int mem_init(void);

unsigned long read_cpu_id(void);

void hang(char err);

#endif

#else  // !CONFIG_GTH

#ifndef __ASSEMBLY__

#include <bitops.h>
#include <task.h>
#include <string.h>
#include <init.h>
#include <malloc.h>
#include <list.h>
#include <font/font.h>

extern init_func_t init_call_begin[], init_call_end[];
extern font_init_t font_init_begin[], font_init_end[];

#endif
#endif
