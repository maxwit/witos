/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

#ifndef __YPORTENV_LINUX_H__
#define __YPORTENV_LINUX_H__

// #include <version.h>
#include <kernel.h>
#include <mm.h>
#include <string.h>
#include <list.h>
#include <types.h>
#include <fs.h>
// #include <stat.h>
// #include <sort.h>
#include <bitops.h>
#include <malloc.h>

/*  These type wrappings are used to support Unicode names in WinCE. */
#define YCHAR char
#define YUCHAR unsigned char
#define _Y(x)     x

#define YAFFS_LOSTNFOUND_NAME		"lost+found"
#define YAFFS_LOSTNFOUND_PREFIX		"obj"


#define YAFFS_ROOT_MODE			0755
#define YAFFS_LOSTNFOUND_MODE		0700

#define Y_CURRENT_TIME 0 // CURRENT_TIME.tv_sec
#define Y_TIME_CONVERT(x) (x).tv_sec

#define compile_time_assertion(assertion) \
	({ int x = __builtin_choose_expr(assertion, 0, (void)0); (void) x; })

#ifdef CONFIG_YAFFS_DEBUG
#define yaffs_trace(msk, fmt, ...) do { \
	if (yaffs_trace_mask & (msk)) \
		printk(KERN_DEBUG "yaffs: " fmt "\n", ##__VA_ARGS__); \
} while (0)
#else
#define yaffs_trace(msk, fmt, ...) do { \
} while (0)
#endif

#endif
