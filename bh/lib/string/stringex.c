#include <types.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


// negative hex skipped
#define MAX_HEX_STR_LEN (sizeof(u32) * 2)

int hex_str_to_val(const char *str, u32 *val)
{
	int iLen = 0;
	u32 tmp = 0;

	while (*str != '\0')
	{
		if (*str >= '0' && *str <= '9')
		{
			tmp <<= 4;
			tmp |= *str - '0';
		}
		else if (*str >= 'a' && *str <= 'f')
		{
			tmp <<= 4;
			tmp |= *str - 'a' + 10;
		}
		else if (*str >= 'A' && *str <= 'F')
		{
			tmp <<= 4;
			tmp |= *str - 'A' + 10;
		}
		else
		{
			return	-EINVAL;
		}

		str++;
		iLen++;
		if (iLen > MAX_HEX_STR_LEN)
			return -EINVAL;
	}

	*val = tmp;

	return iLen;
}


#define MAX_DEC_LEN 32

int val_to_dec_str(char *str, long val)
{
	char buff[MAX_DEC_LEN];
	int i = MAX_DEC_LEN - 1, j = 0;


	if (val < 0)
	{
		str[j++] = '-';
		val = -val;
	}

	while (val)
	{
		buff[i] = val % 10 + '0';
		val /= 10;
		i--;
	}

	while (++i < MAX_DEC_LEN)
	{
		str[j] = buff[i];
		j++;
	}

	str[j] = '\0';

	return j;
}


#define MAX_HEX_LEN (sizeof(u32) * 2 + 1)

int val_to_hex_str(char *str, u32 val)
{
	char buff[MAX_HEX_LEN];
	int i = MAX_HEX_LEN - 1, j = 0;


	while (val)
	{
		u32 num = val & 0xf;

		if (num < 0xa)
		{
			buff[i] = (char)num + '0';
		}
		else
		{
			buff[i] = (char)(num - 0xa) + 'a';
		}

		val >>= 4;
		i--;
	}

	while (++i < MAX_HEX_LEN)
	{
		str[j] = buff[i];
		j++;
	}

	str[j] = '\0';

	return j;
}

int dec_str_to_val(const char *str, u32 *val)
{
	long tmp = 0;
	const char *num = str;


	if ('-' == *num)
	{
		num++;
	}

	while (*num != '\0')
	{
		if (ISDIGIT(*num))
		{
			tmp = 10 * tmp + *num - '0';
		}
		else
		{
			return -EINVAL;
		}

		num++;
	}

	if ('-' == *str)
	{
		*val = -tmp;
	}
	else
	{
		*val = tmp;
	}

	return num - str;
}


/*
 * Convert human string to value. e.g.:
 * "3G40M512K256"
 */
#define G_MARK (1 << 7)
#define M_MARK (1 << 6)
#define K_MARK (1 << 5)

int hr_str_to_val(const char *str, u32 *val)
{
	u8  mark = 0;
	u32 num = 0, tmp = 0;

	while (1)
	{
		switch (*str)
		{
		case '0' ... '9':
			tmp = tmp * 10 + *str - '0';
			break;

		case 'g':
		case 'G':
			if (mark & (G_MARK | M_MARK | K_MARK))
			{
				goto error;
			}

			num += tmp << 30;
			tmp = 0;

			mark |= G_MARK;

			break;

		case 'm':
		case 'M':
			if (mark & (M_MARK | K_MARK))
			{
				goto error;
			}

			num += tmp << 20;
			tmp = 0;

			mark |= M_MARK;

			break;

		case 'k':
		case 'K':
			if (mark & K_MARK)
			{
				goto error;
			}

			num += tmp << 10;
			tmp = 0;

			mark |= K_MARK;

			break;

		case '\0':
			num  += tmp;
			*val = num;
			return 0;

		default:
			goto error;
		}

		str++;
	}

error:
	return -EINVAL;
}


int string2value(const char *str, u32 *val)
{
	int ret;
	u32 tmp;


	if ('0' == *str && ('x' == *(str + 1) || 'X' == *(str + 1)))
	{
		ret = hex_str_to_val(str + 2, &tmp);
	}
	else
	{
		ret = dec_str_to_val(str, &tmp);
	}

	if (ret >= 0)
	{
		*val = tmp;
	}

	return ret;
}


#define KB_MASK ((1 << 10) - 1)

int val_to_hr_str(u32 val, char str[])
{
	char *ch = str;
	u32 sect;
#if 1
	int i;
	struct
	{
		int shift;
		char sign;
	} soff[] = {{30, 'G'}, {20, 'M'}, {10, 'K'}, {0, 'B'}};


	for (i = 0; i < ARRAY_ELEM_NUM(soff); i++)
	{
		sect = (val >> soff[i].shift) & KB_MASK;

		if (sect)
		{
			ch += val_to_dec_str(ch, sect);
			*ch++ = soff[i].sign;
		}
	}
#else

	if (sect = (val >> 30) & KB_MASK)
	{
		ch += val_to_dec_str(ch, sect);
		*ch++ = 'G';
	}

	if (sect = (val >> 20) & KB_MASK)
	{
		ch += val_to_dec_str(ch, sect);
		*ch++ = 'M';
	}

	if (sect = (val >> 10) & KB_MASK)
	{
		ch += val_to_dec_str(ch, sect);
		*ch++ = 'K';
	}

	if (sect = val & KB_MASK)
	{
		ch += val_to_dec_str(ch, sect);
		*ch++ = 'B';
	}
	else if (ch == str)
	{
		*ch++ = '0';
		*ch++ = 'B';
	}
#endif

	*ch = '\0';

	return ch - str;
}


int str_to_ip(u8 ip_val[], const char *ip_str)
{
	int dot = 0;
	unsigned int num = 0;

#if 1
	while (*ip_str)
	{
		if (ISDIGIT(*ip_str))
		{
			num = 10 * num + *ip_str - '0';
		}
		else if ('.' == *ip_str && num <= 255 && dot < 3)
		{
			ip_val[dot] = (u8)num;
			dot++;
			num = 0;
		}
		else
		{
			return -EINVAL;
		}

		ip_str++;
	}

	if (dot < 3 || num > 255)
	{
		return -EINVAL;
	}

	ip_val[dot] = num;

#else
	while (1)
	{
		switch (*ip_str)
		{
		case '0' ... '9':
			num = 10 * num + *ip_str - '0';
			break;

		case '.':
			if (num > 255 || 3 == dot)
			{
				return -EINVAL;
			}

			ip_val[dot] = (u8)num;
			dot++;
			num = 0;
			break;

		case '\0':
			if (3 != dot)
			{
				return -EINVAL;
			}

			ip_val[dot] = num;
			return 0;

		default:
			return -EINVAL;
		}

		ip_str++;
	}
#endif

	return 0;
}


int ip_to_str(char buf[], const u32 ip)
{
	sprintf(buf, "%d.%d.%d.%d",
			ip & 0xff,
			(ip >> 8) & 0xff,
			(ip >> 16) & 0xff,
			(ip >> 24) & 0xff);

	return 0;
}

