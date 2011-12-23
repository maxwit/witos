#!/usr/bin/python

import os, sys
import mw_socs

arch = 'arm'
cpu = 'cortex-a8'
soc = 'OMAP3'
board = 'N900'

def create_defconfig(arch, soc):

	conf_path = 'build/configs/' + arch + '/' + soc.lower() + '_defconfig'

	if os.path.exists(conf_path):
		print '\"' + board + '\" already created! (' + conf_path + ')'
		return 0

	print 'Generate ' + soc.lower() + ' defconfig!\n'
	soc_info_lists = mw_socs.socs_info[soc.upper()]
	soc_info_lists = soc_info_lists.split(' ')
	fd = os.open(conf_path, os.O_RDWR | os.O_CREAT, 0644);
	os.write(fd, '# ' + soc + ' Platform Configuration\n\n');
	os.write(fd, 'CONFIG_' + soc.upper() + ' = y\n' \
				'CONFIG_ARCH = ' + arch + '\n' \
				'CONFIG_ARCH_VER = ' + soc_info_lists[2] + '\n' \
				'CONFIG_LOADER_MENU = y\n' \
				'CONFIG_IRQ_SUPPORT = y\n' \
				'\n' \
				'CONFIG_CROSS_COMPILE = arm-maxwit-linux-gnueabi-\n' \
				'\n' \
				'CONFIG_LOADER_MENU = y\n' \
				'CONFIG_GTH_WRITE = y\n' \
				'\n' \
				'CONFIG_GTH_START_MEM = ' + soc_info_lists[3] + '\n' \
				'CONFIG_GBH_START_MEM = ' + soc_info_lists[4] + '\n' \
				'CONFIG_GBH_START_BLK = 1\n' \
				'\n' \
				'# Flash\n' \
				'CONFIG_NAND = y\n' \
				'CONFIG_NAND_ECC_MODE = NAND_ECC_SW\n' \
				'\n' \
				'# UART\n' \
				'CONFIG_UART = y\n' \
				'CONFIG_UART_INDEX = 0\n' \
				'CONFIG_YMODEM_SUPPORT = y\n' \
				'CONFIG_KERMIT_SUPPORT = y\n' \
				'CONFIG_CONSOLE_NAME = "ttySAC"\n' \
				'\n' \
				'# SPI\n' \
				'\n' \
				'# Graphics\n' \
				'CONFIG_GPU = y\n' \
				'CONFIG_LCD_ID = 0\n' \
				'CONFIG_BOOTUP_LOGO = y\n' \
				'\n' \
				'CONFIG_DM9000 = y\n' \
				'CONFIG_MAC_ADDR = {0x10,0x20,0x30,0x40,0x50,0x60}\n' \
				'CONFIG_LOCAL_IP = MKIP(192,168,2,100)\n' \
				'CONFIG_SERVER_IP = MKIP(192,168,2,101)\n' \
				'CONFIG_NET_MASK = MKIP(255,255,255,0)\n' \
				'CONFIG_IMAGE_PATH = "/var/lib/tftpboot"\n' \
				)
	os.close(fd)
	return 1

if __name__ == "__main__":
	soc_path = 'bh/' + arch + '/' + soc.lower() #fix soc-->plat
	if not os.path.exists(soc_path):
		os.mkdir(soc_path)
	board_path = soc_path + '/' + board.lower()
	if not os.path.exists(board_path):
		os.mkdir(board_path)
	else:
		print '\"' + board + '\" already created! (' + board_path  + ')'

	# create defconfig
	create_defconfig(arch, soc)
