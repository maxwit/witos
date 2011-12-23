#include <sysconf.h>
#include <net/net.h>
#include <flash/flash.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// sys config image size
#define SYSCONF_SIZE(config) ((config)->offset + (config)->size)

static char *g_sysconf;
static bool g_conf_dirty = false;

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
	const char *data = g_sysconf;

	for (i = 0; data[i] != EOF; i++) {
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

	while (*q != EOF) {
		*p = *q;
		p++;
		q++;
	}

	*p = EOF;

	g_conf_dirty = true;

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

	p = g_sysconf;
	while (*p != EOF) p++;

	len = sprintf(p, "%s = %s\n", attr, val);

	p[len] = EOF;

	g_conf_dirty = true;

	return 0;
}

int conf_set_attr(const char *attr, const char *val)
{
	char *p, *q;
	int cur_len, new_len;
	int t;
	int size;

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

	size = 0;
	while (g_sysconf[size] != EOF) size++;

	if (t != 0) {
		memmove(q + t, q, size - (q - g_sysconf));
		g_sysconf[size + t] = EOF;
	}

	memcpy(p, val, new_len);
	g_conf_dirty = true;

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

#ifdef CONFIG_CONSOLE_NAME
	conf_check_add("console", CONFIG_CONSOLE_NAME);
#endif

	return 0;
}

int conf_load()
{
	__u32 *sys_magic = (__u32 *)CONFIG_SYS_START_MEM;
	g_sysconf = (char *)CONFIG_SYS_START_MEM + 4;

	if (GB_SYSCFG_MAGIC != *sys_magic)
		return -EINVAL;

	conf_check_default();

	return 0;
}

// fixme: to support other storage, such as MMC, ATA, ...
int conf_store()
{
	int ret;
	__u32 conf_base = 0, conf_size;
	struct flash_chip *flash;
	char *c;

	if (!g_conf_dirty)
		return 0;

	// fixme
#warning
	flash = flash_open("mtdblock3");
	if (NULL == flash) {
		printf("Fail to open flash!\n");
		return -ENODEV;
	}

	// conf_base = flash->erase_size * CONFIG_SYS_START_BLK;
	conf_size = 0;
	c = g_sysconf;
	while (c[conf_size] != 0xFF) {
		conf_size++;
	}

	ret = flash_erase(flash, conf_base, conf_size, EDF_ALLOWBB);
	if (ret < 0)
		goto L1;

	ret = flash_write(flash, g_sysconf, conf_size, conf_base);
	// if ret < 0 ...

	g_conf_dirty = false;

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

	p = g_sysconf;
	i = j = 0;
	while (p[i] != EOF) {
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
	*(__u32 *)(g_sysconf - 4) = G_SYS_MAGIC;
	*g_sysconf = EOF;

	conf_check_default();

	g_conf_dirty = true;
}
