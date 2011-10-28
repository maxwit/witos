CFLAGS += -include shell.h

dir-y += basic user mem go uart reboot
dir-$(CONFIG_FLASH) += flash
dir-$(CONFIG_NET) += ipconfig ping tftp dhclient ftp
#dir-$(CONFIG_FS) += mount
#dir-y += timer
dir-$(CONFIG_MMC) += disk
dir-$(CONFIG_DISPLAY) += display
