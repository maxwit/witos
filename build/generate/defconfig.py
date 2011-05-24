#!/usr/bin/python
#
# The Main Program for Building Embedded Development Environment
#
# http://www.maxwit.com
# http://maxwit.github.com
# http://maxwit.googlecode.com
#

import os, sys, re
from xml.etree import ElementTree

config = {}

def traverse(node):
	if node.tag == 'choice':
		lst = node.getchildren()

		# fixme: check node ... from .config
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
		l2 = node.text.split(' ')
		for s in l2:
			config[s] = 'y'
	else:
		dfs = True
		if node.tag == 'config':
			if node.attrib.has_key('name'):
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
	
		if dfs == True:
			lst = node.getchildren()
			for n in lst:
				traverse(n)

def parse_tree(fn):
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

	fn = 'build/configs/arm/' + sys.argv[1]
	try:
		fcfg = open(fn)
	except:
		print 'fail to open .config'
		parse_tree("build/configs/configs.xml")
		exit(0)

	for line in fcfg:
		if re.match(r'^CONFIG_', line) <> None:
			elem = re.split('\s*=\s*', re.sub('^CONFIG_', '', line.replace('\n','')))
			config[elem[0]] = elem[1]

	fcfg.close()

	parse_tree("build/configs/configs.xml")

	for x in config:
		print 'CONFIG_' + x + ' = ' + config[x]
