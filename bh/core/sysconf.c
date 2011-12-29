#include <net/net.h>
#include <flash/flash.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define LINE_LEN 512

struct sysconfig {
	char* const data;
	bool is_dirty;
	size_t size;
	int offset;
};

extern char _g_sysconfig[];
extern unsigned int _g_sysconfig_len;

static struct sysconfig g_config = {
	.data = _g_sysconfig,
	.is_dirty = false,
};

static struct sysconfig *_syscfg_get()
{
	return &g_config;
}

static struct sysconfig *_syscfg_open()
{
	struct sysconfig *cfg;

	cfg = _syscfg_get();
	cfg->offset = 0;

	return cfg;
}

static int _syscfg_close(struct sysconfig *cfg)
{
	cfg->offset = 0;

	return 0;
}

static int _syscfg_read_line(struct sysconfig *cfg, char line[], size_t line_len)
{
	int i;
	const char *base;
	size_t size;

	if (cfg == NULL)
		return -ENODEV;

	base = cfg->data + cfg->offset;
	size = cfg->size - cfg->offset;

	if (size == 0)
		return -ENODATA;

	for (i = 0; i < size && i < line_len; i++) {
		if (base[i] == '\n')
			break;

		line[i] = base[i];
	}

	line[i] = '\0';

	if (base[i] == '\n')
		cfg->offset += i + 1;
	else
		cfg->offset += i;

	return i;
}

static int _is_attr_string(const char *attr, const char *string)
{
	const char *p = string;
	size_t len = strlen(attr);

	while (*p) {
		if (!strncmp(attr, p, len)) {
			if (p[len] == ' ' || p[len] == '=')
				return 1;
		}

		p++;
	}

	return 0;
}

static int search_attr(struct sysconfig *cfg, const char *str)
{
	char line[LINE_LEN];
	int ret;

	while ((ret = _syscfg_read_line(cfg, line, LINE_LEN)) >= 0) {
		if (_is_attr_string(str, line)) {
			return ret;
		}
	}

	return -ENODATA;
}

int conf_del_attr(const char *attr)
{
	struct sysconfig *cfg;
	int ret = 0;
	int len;

	cfg = _syscfg_open();

	ret = search_attr(cfg, attr);
	if (ret < 0) {
		DPRINT("Attribute \"%s\" is not exist, del attr error!\n", attr);
		goto L1;
	}

	len = ret + 1;

	memcpy(cfg->data + cfg->offset - len, cfg->data + cfg->offset, cfg->size - cfg->offset);

	cfg->size -= len;

	cfg->is_dirty = true;
L1:
	_syscfg_close(cfg);

	return ret;
}

int conf_add_attr(const char *attr, const char *val)
{
	struct sysconfig *cfg;
	int ret = 0;

	cfg = _syscfg_open();

	if (search_attr(cfg, attr) > 0) {
		DPRINT("Fail to add attribute \"%s\"! (already exists)\n", attr);
		ret = -EBUSY;
		goto L1;
	}

	cfg->size += sprintf(cfg->data + cfg->offset, "%s = %s\n", attr, val);

	cfg->is_dirty = true;

L1:
	_syscfg_close(cfg);

	return ret;
}

int conf_set_attr(const char *attr, const char *val)
{
	int old_len, new_len;
	struct sysconfig *cfg;
	char line[LINE_LEN];
	int ret = 0;

	cfg = _syscfg_open();

	ret = search_attr(cfg, attr);
	if (ret < 0) {
		DPRINT("Attribute \"%s\" does not exists, set attr error\n", attr);
		goto L1;
	}

	old_len = ret + 1; // add  '\n'

	new_len = snprintf(line, sizeof(line), "%s = %s\n", attr, val);
	printf("new attr = %s\n", line);

	if (new_len != old_len) {
		memmove(cfg->data + cfg->offset - old_len + new_len,
			cfg->data + cfg->offset, cfg->size - cfg->offset);
		cfg->size += new_len - old_len;
	}

	memcpy(cfg->data + cfg->offset - old_len, line, new_len);

	cfg->is_dirty = true;

L1:
	_syscfg_close(cfg);

	return ret;
}

// TODO: add ex version:
// int conf_get_attr_ex(char val[], const char *fmt, ...)

int conf_get_attr(const char *attr, char val[])
{
	const char *start, *end;
	struct sysconfig *cfg;
	char line[LINE_LEN];
	int ret = 0;

	cfg = _syscfg_open();

	ret = search_attr(cfg, attr);
	if (ret < 0) {
		DPRINT("Attribute \"%s\" does not exists.\n", attr);
		goto L1;
	}

	cfg->offset -= ret + 1;
	_syscfg_read_line(cfg, line, sizeof(line));
	start = strchr(line, '=');
	start++;

	while (*start == ' ') start++;
	if (*start == '"') start++;

	end = line + strlen(line);
	while (end > line && *end == ' ') end--;

	if (*end == '"') end--;

	while (start <= end) {
		*val++ = *start;
		start++;
	}

	*val = '\0';

L1:
	_syscfg_close(cfg);

	return ret < 0 ? ret : 0;
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
	struct sysconfig *cfg = _syscfg_get();

	if (GB_SYSCFG_MAGIC != *(__u32 *)cfg->data)
		return -EINVAL;

	cfg->size = _g_sysconfig_len;
	DPRINT("sysconf: base = 0x%p, size = %d\n", cfg->data, cfg->size);

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
	struct sysconfig *cfg = _syscfg_get();

	if (!cfg->is_dirty)
		return 0;

	// fixme
	flash = flash_open("mtdblock2");
	if (NULL == flash) {
		printf("Fail to open flash!\n");
		return -ENODEV;
	}

	conf_base = cfg->data - _start;

	ret = flash_erase(flash, conf_base, cfg->size, EDF_ALLOWBB);
	if (ret < 0)
		goto L1;

	ret = flash_write(flash, cfg->data, cfg->size, conf_base);
	if (ret < 0)
		goto L1;

	cfg->is_dirty = false;

L1:
	flash_close(flash);
	return ret;
}

int conf_list_attr()
{
	struct sysconfig *cfg;
	char line[LINE_LEN];

	cfg = _syscfg_open();

	while (_syscfg_read_line(cfg, line, sizeof(line)) >= 0) {
		printf("%s\n", line);
	}

	return 0;
}

void conf_reset(void)
{
	struct sysconfig *cfg = _syscfg_get();

	*(__u32 *)cfg->data = GB_SYSCFG_MAGIC;
	cfg->size = 4;

	conf_check_default();

	cfg->is_dirty = true;
}
