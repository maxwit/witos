
#define FN_MASK 0xff000000

const char* get_func_name(const void *func)
{
	const unsigned long *magic = (const unsigned long *)func - 1;

	if ((*magic & FN_MASK) != FN_MASK)
	    return NULL;

	return (const char *)magic - (*magic & ~FN_MASK);
}
