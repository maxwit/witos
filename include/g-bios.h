#pragma once

#include <autoconf.h>

#ifdef CONFIG_NORMAL_SPACE
#warning "depricated"
#endif

// fixme!
#define CONFIG_HEAD_SIZE 0x2000

// fixme
#define FILE_NAME_SIZE   256

#ifndef __ASSEMBLY__
#include <sysconf.h>

#define VA(x) ((void *)(x))
#endif
