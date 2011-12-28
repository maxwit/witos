#!/usr/bin/python
#
# Usage:
#   $0 .config autoconf.h

import os, sys, string
import mw_socs

def get_plat():
	for soc in mw_socs.socs_info.keys():
		ret = os.system('grep CONFIG_' + soc + ' .config > /dev/null 2>&1')
		if ret == 0:
			soc_list = mw_socs.socs_info[soc].split(' ')
			return soc_list[5] + ' ' + soc_list[0]
	return "null"

def generate_autoconf(dot_conf, auto_conf, plat_info):
	fd1 = open(dot_conf, 'r')
	fd2 = open(auto_conf, 'w')
	plat_info = string.split(plat_info, ' ')
	plat = plat_info[0]	
	arch = plat_info[1]
	# check

	fd2.write("#pragma once\n//\n")

	while 1:
		str = fd1.readline()
		length = len(str)
		if length == 0:
			break;
		if length > 1 and str[0] != '#':
			str = string.replace(str, ' ', '')
			str = str.split('=')
			if len(str) == 2 and str[1] == "y\n":
				fd2.write('#define ' + str[0] + '\n')
			else:
				fd2.write('#define ' + str[0] + ' ' + str[1]);
		elif length == 1:
			fd2.write(str);

	fd2.write('\n//\n')
	fd2.write("#include <" + arch +"/cpu.h>\n")
	fd2.write("#include <" + arch + '/' + plat + ".h>\n")
	fd2.close();
	fd1.close();


if __name__ == "__main__":
	if len(sys.argv) != 3:
		print "usage: " + sys.argv[0] + ' .config autoconf.h'
		sys.exit()

	dot_config = sys.argv[1]
	auto_config= sys.argv[2]

	plat = get_plat()
	if plat == "null":
		print "Platform not specified!"
		sys.exit()

	generate_autoconf(dot_config, auto_config, plat)
