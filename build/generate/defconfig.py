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

	if config.has_key('MAC_ADDR') == False:
		mac1 = hex(random.randint(0, 255))[2:]
		mac2 = hex(random.randint(1, 255))[2:]
		mac3 = hex(random.randint(1, 255))[2:]
		mac4 = hex(random.randint(1, 255))[2:]
		mac5 = hex(random.randint(1, 255))[2:]
		config['MAC_ADDR'] = '"10:' + mac1 + ':' + mac2 + ':' + \
							mac3 + ':' + mac4 + ':' + str(mac5) + '"'

	# fixme:
	# (1) netmask issue;
	# (2) assert(local != server)
	if config.has_key('SERVER_IP') == False:
		server_ip = get_ip_address('eth0') # fixme
		config['SERVER_IP'] = '"' + server_ip + '"'
		local_ip = server_ip.rsplit(".", 1)[0] + "." + str(random.randint(1, 254))
		config['LOCAL_IP'] = '"' + local_ip + '"'
		config['NET_MASK'] = "\"255.255.255.0\""

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
