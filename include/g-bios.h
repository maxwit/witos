#pragma once

#include <autoconf.h>

#ifdef CONFIG_NORMAL_SPACE
#warning "depricated"
#endif

#define MKFOURCC(a, b, c, d)    (((a) << 24) | (b) << 16 | ((c) << 8) | (d))

// fixme
#define FILE_NAME_SIZE   256

#ifndef __ASSEMBLY__
#include <sysconf.h>

#define VA(x) ((void *)(x))
#endif
