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

		if config.has_key(node.attrib['name']):
			default_node = config[node.attrib['name']]
		else:
			if node.attrib.has_key('default'):
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
			try: # if node.attrib.has_key('name'): # if -> try:
				key = node.attrib['name']
				if config.has_key(key) == False:
					if node.attrib.has_key('string'):
						config[key] = '"' + node.attrib['string'] + '"'
					elif node.attrib.has_key('bool'):
						if node.attrib['bool'] == "y":
							config[key] = node.attrib['bool']
						else:
							dfs = False
					elif node.attrib.has_key('int'):
						config[key] = node.attrib['int']
					else:
						print 'Invalid node!'
						print node
			except:
				print 'Invalid node!'
				print node

		if dfs == True:
			lst = node.getchildren()
			for n in lst:
				traverse(n)

def get_ip_address(ifname):
	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

	try:
		local_ip = fcntl.ioctl(s.fileno(), 0x8915, # SIOCGIFADDR
					struct.pack('256s', ifname[:15]))
	except IOError:
		return "10.0.0.1"

	return socket.inet_ntoa(local_ip[20:24])

def parse_config(fn):
	try:
		tree = ElementTree.parse(fn)
	except:
		print "fail to parse " + fn + "!"
		exit(1)

	root = tree.getroot()
	lst = root.getchildren()
	for n in lst:
		traverse(n)

def get_attr(substr, fd):
	for line in fd:
		if re.match(substr, line) <> None:
			elem = re.split('\s*=\s*', line.replace('\n',''))
			return elem[1]
	return None

#fixme
def get_active_nic():
	return "eth0"

#fixme
def get_net_mask(nic):
	return "255.255.255.0"

def generate_netmask(ip):
	return "255.255.255.0"

def parse_sysconfig(sys_cfg_file):
	sysconfig = {}
	try:
		sys_cfg_fd = open(sys_cfg_file, 'aw+')
	except:
		print 'fail to open "%s"' % sys_cfg_file
		exit(1)

	attr = get_attr('net.eth0.method', sys_cfg_fd)
	if attr == "dhcp":
		return

	attr = get_attr('net.server', sys_cfg_fd);
	if attr == None:
		nic = get_active_nic()
		sysconfig['net.server'] = get_ip_address(nic)
		server = sysconfig['net.server'];
		def_netmask = get_net_mask(nic)
	else:
		server = attr

	attr = get_attr('net.eth0.netmask', sys_cfg_fd)
	if attr == None:
		if def_netmask <> None:
			sysconfig['net.eth0.netmask'] = def_netmask
		else:
			sysconfig['net.eth0.netmask'] = generate_netmask(server)

	attr = get_attr('net.eth0.address', sys_cfg_fd)
	if attr == None:
		address = server
		while address == server:
			address = server.rsplit('.', 1)[0] + '.' + str(random.randint(1, 254))

		sysconfig['net.eth0.address'] = address

	attr = get_attr('net.eth0.gateway', sys_cfg_fd)
	if attr == None:
		sysconfig['net.eth0.gateway'] = server

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
		print 'cannot run as root!'
		exit(1)

	if len(sys.argv) <> 2:
		print "Usage: ..."
		exit(1)

	fn_def_cfg = 'build/configs/arm/' + sys.argv[1]

	try:
		fd_def_cfg = open(fn_def_cfg)
	except:
		print 'fail to open "%s"' % fn_def_cfg
		# parse_config("build/configs/configs.xml")
		exit(1)

	for line in fd_def_cfg:
		if re.match(r'^CONFIG_', line) <> None:
			elem = re.split('\s*=\s*', re.sub('^CONFIG_', '', line.replace('\n','')))
			config[elem[0]] = elem[1]

	fd_def_cfg.close()

	parse_config("build/configs/configs.xml")

	# print config
	# print 'configure %s (%s).' % (config['BOARD'], config['ARCH'])

	try:
		fd_dot_cfg = open('.config', 'w')
	except:
		print 'fail to open .config file'
		exit(1)

	for x in config:
		fd_dot_cfg.write('CONFIG_' + x + ' = ' + config[x] + '\n')

	fd_dot_cfg.close()

	parse_sysconfig('.sysconfig')
