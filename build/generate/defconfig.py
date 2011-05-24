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

def traverse(node, prefix=None):
	if node.tag == 'choice':
		lst = node.getchildren()

		# check node ... from .config
		if node.attrib.has_key('default'):
			default_node = node.attrib['default'].upper()
		else:
			default_node = lst[0].attrib['name'].upper()

		for n in lst:
			if n.attrib['name'].upper() == default_node:
				traverse(n, node.attrib['name'].upper())
	elif node.tag == 'select':
		l2 = node.text.split(' ')
		for s in l2:
			print 'CONFIG_' + s + ' = y'
	else:
		if node.tag == 'config':
			if node.attrib.has_key('name'):
				name = node.attrib['name'].upper()
				if prefix <> None:
					name = prefix + '_' + name
				if node.attrib.has_key('string'):
					print 'CONFIG_%s = "%s"' % (name, node.attrib['string'])
				elif node.attrib.has_key('bool'):
					print 'CONFIG_%s = %s' % (name, node.attrib['bool'])
				elif node.attrib.has_key('int'):
					print 'CONFIG_%s = %s' % (name, node.attrib['int'])
	
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

	parse_tree("build/configs/configs.xml")
