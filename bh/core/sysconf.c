#include <net/net.h>
#include <flash/flash.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct sysconfig {
	char* const data;
	bool is_dirty;
	size_t size;
};

extern char _g_sysconfig[];
extern unsigned int _g_sysconfig_len;

static struct sysconfig g_config = {
	.data = _g_sysconfig,
	.is_dirty = false,
};

__u32 get_load_mem_addr()
{
	return 0;
}

void set_load_mem_addr(__u32 *addr)
{
	return;
}

static char *search_attr(const char *str)
{
	int i, j;
	int len = strlen(str);
	int is_attr = 1;
	const char *data = g_config.data;

	for (i = 0; i < g_config.size; i++) {
		if (is_attr) {
			if (data[i] != ' ') {
				for (j = 0; j < len && data[i + j] == str[j]; j++);

				if (j == len && (data[i + j] == ' ' || data[i + j] == '=')) {
					return (char *)data + i;
				}

				is_attr = 0;
			}
		}

		if (data[i] == '\n')
			is_attr = 1;
	}

	return NULL;
}

int conf_del_attr(const char *attr)
{
	char *p, *q;

	p = search_attr(attr);
	if (p == NULL) {
		DPRINT("Attribute \"%s\" is not exist, del attr error!\n", attr);
		return -1;
	}

	q = strchr(p, '\n');
	q++;

	while (q < g_config.data + g_config.size) {
		*p = *q;
		p++;
		q++;
	}

	g_config.size = p - g_config.data;

	g_config.is_dirty = true;

	return 0;
}

int conf_add_attr(const char *attr, const char *val)
{
	char *p;
	int len;

	p = search_attr(attr);
	if (p != NULL) {
		DPRINT("Fail to add attribute \"%s\"! (already exists)\n", attr);
		return -EBUSY;
	}

	p = g_config.data + g_config.size;

	len = sprintf(p, "%s = %s\n", attr, val);
	g_config.size += len;

	g_config.is_dirty = true;

	return 0;
}

int conf_set_attr(const char *attr, const char *val)
{
	char *p, *q;
	int cur_len, new_len;
	int t;

	p = search_attr(attr);
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
		memmove(q + t, q, g_config.size - (q - g_config.data));
		g_config.size += t;
	}

	memcpy(p, val, new_len);
	g_config.is_dirty = true;

	return 0;
}

int conf_get_attr(const char *attr, char val[])
{
	const char *p;

	p = search_attr(attr);
	if (p == NULL) {
		DPRINT("Attribute \"%s\" is not exist, get attr error!\n", attr);
		return -ENOENT;
	}

	p = strchr(p, '=');
	p++;

	while (*p != '\n') {
		if (*p != ' ')
			*val++ = *p;

		p++;
	}

	*val = '\0';

	return 0;
}

// fixme
static inline void conf_check_add(const char *attr, const char *val)
{
	char str[CONF_VAL_LEN];

	if (conf_get_attr(attr, str) < 0)
		conf_add_attr(attr, val);
}

static int conf_check_default()
{
#ifdef CONFIG_SERVER_IP
	conf_check_add("net.server", CONFIG_SERVER_IP);
#endif

#ifdef CONFIG_LOCAL_IP
	conf_check_add("net.eth0.address", CONFIG_LOCAL_IP);
#endif

#ifdef CONFIG_NET_MASK
	conf_check_add("net.eth0.netmask", CONFIG_NET_MASK);
#endif

#ifdef CONFIG_MAC_ADDR
	conf_check_add("net.eth0.mac", CONFIG_MAC_ADDR);
#endif

#if 0
#ifdef CONFIG_CONSOLE_NAME
	conf_check_add("console", CONFIG_CONSOLE_NAME);
#endif
#endif

	return 0;
}

int conf_load()
{
	__u32 *sys_magic = (__u32 *)_g_sysconfig;

	if (GB_SYSCFG_MAGIC != *sys_magic)
		return -EINVAL;

	g_config.size = _g_sysconfig_len;
	DPRINT("sysconf: base = 0x%p, size = %d\n", g_config.data, g_config.size);

	conf_check_default();

	return 0;
}

// fixme: to support other storage, such as MMC, ATA, ...
int conf_store()
{
	int ret;
	__u32 conf_base;
	struct flash_chip *flash;
	extern char _start[];

	if (!g_config.is_dirty)
		return 0;

	// fixme
	flash = flash_open("mtdblock2");
	if (NULL == flash) {
		printf("Fail to open flash!\n");
		return -ENODEV;
	}

	conf_base = _g_sysconfig - _start;

	ret = flash_erase(flash, conf_base, g_config.size, EDF_ALLOWBB);
	if (ret < 0)
		goto L1;

	ret = flash_write(flash, g_config.data, g_config.size, conf_base);
	if (ret < 0)
		goto L1;

	g_config.is_dirty = false;

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

	p = g_config.data;
	i = j = 0;
	while (i < g_config.size) {
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

void conf_reset(void)
{
	*(__u32 *)(g_config.data - 4) = GB_SYSCFG_MAGIC;
	g_config.size = 4;

	conf_check_default();

	g_config.is_dirty = true;
}
