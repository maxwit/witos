#include <stdio.h>


#define FN_MASK 0xff000000


const char* get_func_name(const void *func)
{
	const u32 *pMagic = (const u32 *)func - 1;
    u32 dwMagic = *pMagic;

    if ((dwMagic & FN_MASK) != FN_MASK)
        return NULL;

    return (const char *)pMagic - (dwMagic & ~FN_MASK);
}

