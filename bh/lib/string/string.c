#include <string.h>
#include <types.h>

size_t strlen(const char *psrc)
{
	const char *iter;

	for (iter = psrc; *iter; iter++);

	return iter - psrc;
}

size_t strnlen(const char *psrc, size_t count)
{
	const char *iter;

	for (iter = psrc; *iter && count; iter++, count--);

	return iter - psrc;
}

char *strcpy(char *pdst, const char *psrc)
{
	char *iter = pdst;

	while ((*iter = *psrc) != '\0')
	{
		iter++;
		psrc++;
	}

	return pdst;
}

char *strncpy(char *pdst, const char *psrc, size_t count)
{
	char *iter = pdst;
	size_t n = 0;

	while ((n < count) && (*iter = *psrc))
	{
		iter++;
		psrc++;
		n++;
	}

	while (n < count)
	{
		*iter++ = '\0';
		n++;
	}

	return pdst;
}

int strcmp (const char *pstr1, const char *pstr2)
{
	while (*pstr1 == *pstr2)
	{
		if ('\0' == *pstr1)
			return 0;

		pstr1++;
		pstr2++;
	}

	return *pstr1 - *pstr2;
}

int strncmp(const char *pstr1, const char *pstr2, size_t count)
{
	size_t n = 1;

	while (*pstr1 == *pstr2)
	{
		if (('\0' == *pstr1) || (n == count))
			return 0;

		pstr1++;
		pstr2++;
		n++;
	}

	return *pstr1 - *pstr2;
}

// fixme!!
int strcasecmp (const char *pstr1, const char *pstr2)
{
	while (*pstr1 == *pstr2)
	{
		if ('\0' == *pstr1)
			return 0;

		pstr1++;
		pstr2++;
	}

	return *pstr1 - *pstr2;
}

char *strcat(char *pdst, const char *psrc)
{
	char *iter;

	for (iter = pdst; *iter; iter++);

	while ((*iter = *psrc) != '\0')
	{
		iter++;
		psrc++;
	}

	return pdst;
}

char *strncat(char *pdst, const char *psrc, size_t count)
{
	char *iter;
	size_t n = 0;

	for (iter = pdst; *iter; iter++);

	while (n < count && (*iter = *psrc))
	{
		iter++;
		psrc++;
		n++;
	}

	while (n < count)
	{
		*iter = '\0';
		iter++;
		n++;
	}

	return pdst;
}

char *strchr(const char *psrc, int c)
{
	const char *iter;

	for (iter = psrc; *iter; iter++)
	{
		if (*iter == c)
			return (char *)iter;
	}

	return NULL;
}

char *strrchr(const char *psrc, size_t c)
{
	const char *iter;

	for (iter = psrc; *iter; iter++);

	while (iter > psrc)
	{
		iter--;
		if (*iter == c)
			return (char *)iter;
	}

	return NULL;
}

void *memcpy(void *pdst, const void *psrc, size_t count)
{
	u8 *pd;
	const u8 *ps;

	pd = pdst;
	ps = psrc;

	while (count > 0)
	{
		*pd++ = *ps++;
		count--;
	}

	return pdst;
}

void *memmove(void *pdst, const void *psrc, size_t count)
{
	u8 *pd;
	const u8 *ps;

	if (pdst < psrc)
	{
		pd = pdst;
		ps = psrc;

		while (count > 0)
		{
			*pd++ = *ps++;
			count--;
		}
	}
	else
	{
		pd = pdst + count;
		ps = psrc + count;

		while (count > 0)
		{
			*--pd = *--ps;
			count--;
		}
	}

	return pdst;
}

void *memset(void *psrc, int c, size_t count)
{
	char *ps = psrc;

	while (count)
	{
		*ps = c;

		ps++;
		count--;
	}

	return psrc;
}

long memcmp(const void* pdst, const void* psrc, size_t count)
{
	const u8 *ps, *pd;

	pd = pdst;
	ps = psrc;

	while (count > 0)
	{
		if (*pd != *ps)
			return *pd - *ps;

		ps++;
		pd++;

		count--;
	}

	return 0;
}

