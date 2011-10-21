#pragma once

typedef unsigned char  __u8;
typedef unsigned short __u16;
typedef unsigned int   __u32, size_t;
typedef signed int     ssize_t;
typedef enum {false, true} bool;

#define WORD_SIZE              sizeof(__u32)
#define WORD_ALIGN_UP(addr)    (((addr) + WORD_SIZE - 1) & ~(WORD_SIZE - 1))
#define WORD_ALIGN_DOWN(addr)  ((addr) & ~(WORD_SIZE - 1))
#define DWORD_SIZE             (WORD_SIZE << 1)

#define cpu_to_le16(x) (x)
#define le16_to_cpu(x) (x)

#define CPU_TO_BE16(val)	 (((val) >> 8) | (((val) & 0xff) << 8))
#define BE16_TO_CPU(val)	 (((val) >> 8) | (((val) & 0xff) << 8))

#define BE32_TO_CPU(val)  ((((val) >> 24) & 0xff) | (((val) >> 8) & 0xff00) | ((((val) << 8) & 0xff0000)) | ((val) << 24))
#define CPU_TO_BE32(val)  ((((val) >> 24) & 0xff) | (((val) >> 8) & 0xff00) | ((((val) << 8) & 0xff0000)) | ((val) << 24))

// #define ALIGN_UP(len, align) (((len) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_UP(len, align) \
	do { \
		typeof(len) nTemp; \
		if (!((align) & ((align) - 1))) { \
			(len) = (len + align - 1) & ~((align) - 1); \
		} else if ((nTemp = (len) % (align))) { \
			(len) += (align) - nTemp; \
		} \
	} while (0)

