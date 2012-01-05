#pragma once

#include <types.h>

#ifdef __GBIOS_VER__
#define __INIT__           __attribute__ ((__section__(".code.init")))
#else
#define __INIT__           __attribute__((constructor))
#endif

#define __INIT_DATA__      __attribute__ ((__section__(".data.init")))

#define INIT_CALL_LEVEL(n) __attribute__ ((__section__(".Level" #n ".gbios_init")))

#define __INIT_ARCH__     INIT_CALL_LEVEL(0)
#define __INIT_PLAT__     INIT_CALL_LEVEL(1)
#define __INIT_SUBS__     INIT_CALL_LEVEL(2)
#define __INIT_POSTSUBS__ INIT_CALL_LEVEL(3)
#define __INIT_DRV__      INIT_CALL_LEVEL(4)

#define ARCH_INIT(func) \
	static __USED__ __INIT_ARCH__ init_func_t __initcall_##func = func

#define PLAT_INIT(func) \
	static __USED__ __INIT_PLAT__ init_func_t __initcall_##func = func

#define SUBSYS_INIT(func) \
	static __USED__ __INIT_SUBS__ init_func_t __initcall_##func = func

#define POSTSUBS_INIT(func) \
		static __USED__ __INIT_POSTSUBS__ init_func_t __initcall_##func = func

#ifdef __GBIOS_VER__
#define module_init(func) \
	static __USED__ __INIT_DRV__  init_func_t __initcall_##func = func
#else
#define module_init(func)
#endif

typedef int (*init_func_t)(void);

// fixme
const char* get_func_name(const void *func);
