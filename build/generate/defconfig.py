#!/usr/bin/python
#
# The Main Program for Building Embedded Development Environment
#
# http://www.maxwit.com
# http://maxwit.github.com
# http://maxwit.googlecode.com
#

import os, sys, re, random, socket, fcntl, struct
from xml.etree import ElementTree

config = {}

def traverse(node):
	if node.tag == 'choice':
		lst = node.getchildren()

		if node.attrib['name'] in config:
			default_node = config[node.attrib['name']]
		else:
			if 'default' in node.attrib:
				default_node = node.attrib['default']
			else:
				default_node = lst[0].attrib['name']
			config[node.attrib['name']] = default_node

		for n in lst:
			if n.attrib['name'] == default_node:
				traverse(n)
			# fixme: check matched or not
	elif node.tag == 'select':
		l2 = node.text.split(' ') # fixme: re.split
		for s in l2:
			config[s] = 'y'
	else:
		dfs = True
		if node.tag == 'config':
			try: # if 'name' in node.attrib: # if -> try:
				key = node.attrib['name']
				if key in config:
					if 'string' in node.attrib:
						config[key] = '"' + node.attrib['string'] + '"'
					elif 'bool' in node.attrib:
						if node.attrib['bool'] == "y":
							config[key] = node.attrib['bool']
						else:
							dfs = False
					elif 'int' in node.attrib:
						config[key] = node.attrib['int']
					else:
						print('Invalid node!')
						print(node)
			except:
				print('Invalid node!')
				print(node)

		if dfs == True:
			lst = node.getchildren()
			for n in lst:
				traverse(n)

#def get_ip_address(ifname):
#	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#
#	try:
#		local_ip = fcntl.ioctl(s.fileno(), 0x8915, # SIOCGIFADDR
#					struct.pack('256s', ifname[:15]))
#	except IOError:
#		return "10.0.0.1"
#
#	return socket.inet_ntoa(local_ip[20:24])

def parse_config(fn):
	try:
		tree = ElementTree.parse(fn)
	except:
		print("fail to parse " + fn + "!")
		exit(1)

	root = tree.getroot()
	lst = root.getchildren()
	for n in lst:
		traverse(n)

def get_attr(substr, fd):
	for line in fd:
		if re.match(substr, line) != None:
			elem = re.split('\s*=\s*', line.replace('\n',''))
			return elem[1]
	return None

def parse_sysconfig(sys_cfg_file):
	sysconfig = {}

	sys_cfg_fd = open(sys_cfg_file, 'w+')

	attr = get_attr('net.eth0.mac', sys_cfg_fd)
	if attr == None:
		mac1 = hex(random.randint(0, 255))[2:]
		mac2 = hex(random.randint(1, 255))[2:]
		mac3 = hex(random.randint(1, 255))[2:]
		mac4 = hex(random.randint(1, 255))[2:]
		mac5 = hex(random.randint(1, 255))[2:]
		sysconfig['net.eth0.mac'] = '10:' + str(mac1) + ':' + str(mac2) + ':' + \
							str(mac3) + ':' + str(mac4) + ':' + str(mac5)

	for x in sysconfig:
		sys_cfg_fd.write(x + ' = ' + sysconfig[x] + '\n')

	sys_cfg_fd.close()


if __name__ == "__main__":
	if os.getenv('USER') == 'root':
		print('cannot run as root!')
		exit(1)

	if len(sys.argv) != 2:
		print("Usage: ...")
		exit(1)

	fn_def_cfg = 'build/configs/arm/' + sys.argv[1]

	fd_def_cfg = open(fn_def_cfg)

	for line in fd_def_cfg:
		if re.match(r'^CONFIG_', line) != None:
			elem = re.split('\s*=\s*', re.sub('^CONFIG_', '', line.replace('\n','')))
			config[elem[0]] = elem[1]

	fd_def_cfg.close()

	parse_config("build/configs/configs.xml")

	# print(config)
	# print('configure %s (%s).' % (config['BOARD'], config['ARCH']))

	try:
		fd_dot_cfg = open('.config', 'w')
	except:
		print('fail to open .config file')
		exit(1)

	for x in config:
		fd_dot_cfg.write('CONFIG_' + x + ' = ' + config[x] + '\n')

	fd_dot_cfg.close()

	p = re.compile('defconfig')
	fn_sys_cfg = p.sub('board.inf', fn_def_cfg)

	if os.path.exists(fn_sys_cfg):
		os.system('cp -v '+ fn_sys_cfg + ' board.inf')
	else:
		os.system('rm board.inf')

	parse_sysconfig('board.inf')
