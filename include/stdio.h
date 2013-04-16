#pragma once

#include <types.h>

#define printk printf

#define KERN_EMERG	"<0>"	/* system is unusable			*/
#define KERN_ALERT	"<1>"	/* action must be taken immediately	*/
#define KERN_CRIT	"<2>"	/* critical conditions			*/
#define KERN_ERR	"<3>"	/* error conditions			*/
#define KERN_WARNING	"<4>"	/* warning conditions			*/
#define KERN_NOTICE	"<5>"	/* normal but significant condition	*/
#define KERN_INFO	"<6>"	/* informational			*/
#define KERN_DEBUG	"<7>"	/* debug-level messages			*/

int putchar(int);

char *gets(char *);

int puts(const char *);

int printf(const char *, ...);

int sprintf(char *, const char *, ...);

int snprintf(char *, size_t, const char *, ...);

// right here?
#ifdef CONFIG_DEBUG
#define DPRINT(fmt, args ...) \
	printf(fmt, ##args)
#define GEN_DBG(fmt, args ...) \
	printf("%s() line %d: " fmt, __func__, __LINE__, ##args)
#else
#define DPRINT(fmt, args ...)
#define GEN_DBG(fmt, args ...)
#endif
