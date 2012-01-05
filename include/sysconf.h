#pragma once

#define GB_SYSCFG_VER    7
#define GB_SYSCFG_MAGIC  "sysG"

#define __SYSCONF__ __attribute__ ((__section__(".gsect_sysconfig")))

// End of the file
#define EOF 0xFF

#define CONSOLE_DEV_NAME_LEN   63
#define DEFAULT_KCMDLINE_LEN   512

#define BM_MASK        ((1 << 8) - 1)

#define BM_RAMDISK     (1 << 8)
#define BM_FLASHDISK   (2 << 8)
#define BM_NFS         (4 << 8)
#define BM_MMC         (8 << 8)
#define BM_TFTP        (16 << 8)

// standard configuration
#define HOME  "HOME"

#define CONF_VAL_LEN 512
#define CONF_ATTR_LEN 128

// API list
int conf_load(void);
void conf_reset(void);
int conf_get_attr(const char *attr, char val[]);
int conf_set_attr(const char *attr, const char *val);
int conf_add_attr(const char *attr, const char *val);
int conf_del_attr(const char *attr);
int conf_list_attr(void);
int conf_store(void);
