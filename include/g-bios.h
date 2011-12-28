#pragma once

#include <autoconf.h>

#ifdef CONFIG_NORMAL_SPACE
#warning "depricated"
#endif

#ifndef __ASSEMBLY__
#include <types.h>
#include <errno.h>
#include <timer.h>
#include <io.h>
#include <sysconf.h>

void udelay(__u32 n);
void mdelay(__u32 n);

#endif

#include <image.h>

#ifdef CONFIG_GTH

#ifndef __ASSEMBLY__

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
#endif
#endif
