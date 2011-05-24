#!/usr/bin/python
#
# The Main Program for Building Embedded Development Environment
#
# http://www.maxwit.com
# http://maxwit.github.com
# http://maxwit.googlecode.com
#

import os,sys,re
from xml.etree import ElementTree

config = {}

def traverse(node):
	if node.tag == 'choice':
		lst = node.getchildren()

		# fixme: check node ... from .config
		if node.attrib.has_key('default'):
			default_node = node.attrib['default'].upper()
		else:
			default_node = lst[0].attrib['name'].upper()

		for n in lst:
			if n.attrib['name'].upper() == default_node:
				traverse(n)
			# fixme: check matched or not
	elif node.tag == 'select':
		l2 = node.text.split(' ')
		for s in l2:
			print 'CONFIG_' + s + ' = y'
	else:
		dfs = True
		if node.tag == 'config':
			if node.attrib.has_key('name'):
				name = node.attrib['name'].upper()

				if node.attrib.has_key('string'):
					key = 'CONFIG_' + name
					if config.has_key(key) == False:
						config[key] = '"' + node.attrib['string'] + '"'
				elif node.attrib.has_key('bool'):
					if node.attrib['bool'] == "y":
						print 'CONFIG_%s = %s' % (name, node.attrib['bool'])
					else:
						dfs = False
				elif node.attrib.has_key('int'):
					print 'CONFIG_%s = %s' % (name, node.attrib['int'])
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

	try:
		fcfg = open(".config")
	except:
		print 'fail to open .config'
		parse_tree("build/configs/configs.xml")
		exit(0)

	for line in fcfg:
		if re.match(r'^CONFIG_', line) <> None:
			elem = line.replace('\n', '').split('=') #\s+=\s+
			config[elem[0]] = elem[1]

	fcfg.close()

	parse_tree("build/configs/configs.xml")
