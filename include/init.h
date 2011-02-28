#pragma once

#define __INIT__           __attribute__ ((__section__(".code.init")))

#define INIT_CALL_LEVEL(n) __attribute__ ((__section__(".Level" #n ".gbios_init")))

#define __INIT_ARCH__     INIT_CALL_LEVEL(0)
#define __INIT_PLAT__     INIT_CALL_LEVEL(1)
#define __INIT_SUBS__     INIT_CALL_LEVEL(2)
#define __INIT_POSTSUBS__ INIT_CALL_LEVEL(3)
#define __INIT_DRV__      INIT_CALL_LEVEL(4)
#define __INIT_APP__      INIT_CALL_LEVEL(5)


#if __GNUC__ == 3 && __GNUC_MINOR__ >= 3 || __GNUC__ >= 4
#define __USED__    __attribute__((__used__))
#else
#define __USED__    __attribute__((__unused__))
#endif


typedef int (*INIT_FUNC_PTR)(void);

#define ARCH_INIT(func) \
	static __USED__ __INIT_ARCH__ INIT_FUNC_PTR __initcall_##func = func

#define PLAT_INIT(func) \
	static __USED__ __INIT_PLAT__ INIT_FUNC_PTR __initcall_##func = func

#define SUBSYS_INIT(func) \
	static __USED__ __INIT_SUBS__ INIT_FUNC_PTR __initcall_##func = func

#define POSTSUBS_INIT(func) \
		static __USED__ __INIT_POSTSUBS__ INIT_FUNC_PTR __initcall_##func = func

#define DRIVER_INIT(func) \
	static __USED__ __INIT_DRV__  INIT_FUNC_PTR __initcall_##func = func

#define APP_INIT(func) \
	static __USED__ __INIT_APP__  INIT_FUNC_PTR __initcall_##func = func


// fixme
const char* get_func_name(const void *func);

