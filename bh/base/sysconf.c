#include <sysconf.h>
#include <net/net.h>
#include <flash/flash.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static struct sys_config *g_sys;

static u32 conf_checksum(u32 *new_sum)
{
	u32 old_sum = g_sys->checksum;

	g_sys->checksum = 0;
	g_sys->checksum = ~net_calc_checksum(g_sys, g_sys->offset + g_sys->size);
	g_sys->checksum |= GB_SYSCFG_VER << 16;

	if (NULL != new_sum)
		*new_sum = g_sys->checksum;

	return old_sum;
}

static char *search_attr(const char *data, int size, const char *str)
{
	int i, j;
	int len = strlen(str);
	int is_attr = 1;

	for (i = 0; i < size - len; i++) {
		if (is_attr) {
			if (data[i] != ' ') {
				for (j = 0; j < len && data[i + j] == str[j]; j++);

				if (j == len && (data[i + j] == ' ' || data[i + j] == '=')) {
					return (char *)data + i;
				}

				is_attr = 0;
			}
		}

		if (data[i] == '\n') {
				is_attr = 1;
		}
	}

	return NULL;
}

int conf_del_attr(const char *attr)
{
	char *p, *q;
	int mov_len;
	char *sys_data = (char *)g_sys + g_sys->offset;

	p = search_attr(sys_data, g_sys->size, attr);
	if (p == NULL) {
		DPRINT("Attribute \"%s\" is not exist, del attr error!\n", attr);
		return -1;
	}

	q = strchr(p, '\n');
	q++;

	mov_len = g_sys->size - (q - sys_data);
	memmove(p, q, mov_len);

	g_sys->size -= q - p;

	return 0;
}

int conf_add_attr(const char *attr, const char *val)
{
	char *p;
	int len;
	char *sys_data = (char *)g_sys + g_sys->offset;

	p = search_attr(sys_data, g_sys->size, attr);
	if (p != NULL) {
		DPRINT("Attribute \"%s\" is exist, add attr error!\n", attr);
		return -1;
	}

	p = sys_data + g_sys->size;

	len = sprintf(p, "%s = %s\n", attr, val);

	g_sys->size = g_sys->size + len;

	return 0;
}

int conf_set_attr(const char *attr, const char *val)
{
	char *p, *q;
	int cur_len, new_len;
	int t;
	char *sys_data = (char *)g_sys + g_sys->offset;

	p = search_attr(sys_data, g_sys->size, attr);
	if (p == NULL) {
		DPRINT("Attribute \"%s\" is not exist, set attr error\n", attr);
		return -1;
	}

	p = strchr(p, '=');
	p++;
	q = strchr(p, '\n');

	cur_len = q - p;
	new_len = strlen(val);

	t = new_len - cur_len;

	if (t != 0) {
		memmove(q + t, q, g_sys->size - (q - sys_data));
		g_sys->size += t;
	}

	memcpy(p, val, new_len);

	return 0;
}

int conf_get_attr(const char *attr, char val[])
{
	const char *p;
	char *sys_data = (char *)g_sys + g_sys->offset;

	p = search_attr(sys_data, g_sys->size, attr);
	if (p == NULL) {
		DPRINT("Attribute \"%s\" is not exist, get attr error!\n", attr);
		return -1;
	}

	p = strchr(p, '=');
	p++;

	while (*p != '\n') {
		if (*p != ' ') {
			*val++ = *p;
		}

		p++;
	}

	*val = '\0';

	return 0;
}

int conf_load()
{
	u32 new_sum, old_sum;

	g_sys = (struct sys_config*)CONFIG_SYS_START_MEM;

	old_sum = conf_checksum(&new_sum);

	if (old_sum != new_sum)	{
		DPRINT("checksum error! (0x%08x != 0x%08x)\n", new_sum, old_sum);
		return -EINVAL;
	}

	return 0;
}

int conf_store()
{
	int ret;
	u32 conf_base, conf_size;
	struct flash_chip *flash;

	flash = flash_open(BOOT_FLASH_ID);
	if (NULL == flash) {
		return -ENODEV;
	}

	conf_base = flash->erase_size * CONFIG_SYS_START_BLK;
	conf_size = g_sys->size + g_sys->offset;

	ret = flash_erase(flash, conf_base, conf_size, EDF_ALLOWBB);

	if (ret < 0) {
		goto L1;
	}

	conf_checksum(NULL);

	ret = flash_write(flash, g_sys, conf_size, conf_base);

L1:
	flash_close(flash);

	return ret;
}

int conf_list_attr()
{
	char attr[CONF_VAL_LEN];
	char val[CONF_VAL_LEN];
	int i, j;
	char *p;
	int is_attr = 1;
	char *sys_data = (char *)g_sys + g_sys->offset;

	p = sys_data;
	i = j = 0;
	while (i < g_sys->size) {
		if (is_attr) {
			if (p[i] != ' ' && p[i] != '=') {
				attr[j++] = p[i];
			} else if (p[i] == '=') {
				is_attr = 0;
				attr[j] = '\0';
				printf("%s = ", attr);
				j = 0;
			}
		} else {
			if (p[i] != ' ' && p[i] != '\n') {
				val[j++] = p[i];
			} else if (p[i] == '\n') {
				is_attr = 1;
				val[j] = '\0';
				printf("%s\n", val);
				j = 0;
			}
		}

		i++;
	}

	return 0;
}

int __INIT__ sysconf_init(void)
{
	int ret;

	ret = conf_load();

	// ...

	return ret;
}
