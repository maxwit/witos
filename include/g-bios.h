#pragma once

#include <autoconf.h>

#define GTH_MAGIC    (('G' << 24) | ('B' << 16) | (('t'- 'a') << 8) | 'h')
#define GBH_MAGIC    (('G' << 24) | ('B' << 16) | (('b'- 'a') << 8) | 'h')

#define HEAP_SIZE   (8 << 20)


#ifndef __ASSEMBLY__
#include <types.h>
#include <errno.h>
#include <timer.h>
#include <io.h>
#endif

#ifdef CONFIG_GTH

#ifndef __ASSEMBLY__

#define GBH_LOAD_SIZE     KB(512)

int cpu_init(void);

int soc_init(void);

int uart_init(void);

int mem_init(void);

// int init_mmc(struct mmc *);

int printf(const char *fmt, ...);

u32 read_cpu_id(void);

void hang(char err);

#endif

#else  // CONFIG_GTH

#ifndef __ASSEMBLY__

#include <bitops.h>
#include <app.h>
#include <stdio.h>
#include <string.h>
#include <init.h>
#include <malloc.h>
#include <list.h>

extern INIT_FUNC_PTR init_call_begin[], init_call_end[];

extern u8 *g_pDefLoadAddr;

#endif
#endif
