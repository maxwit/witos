#!/usr/bin/python

import sys
import os
import getopt
import re

# switch class
class switch(object):
	def __init__ (self, value):
		self.value=value
		self.fall=False

	def __iter__ (self):
		yield self.match
		raise StopIteration

	def match (self, *args):
		if self.fall or not args:
			return True
		elif self.value in args:
			self.fall=True
			return True
		else:
			return False

# make image class
class nand_image:
	def __init__ (self):
		self.flash_image_name = ""

		# default size
		self.flash_page_size   = 2048
		self.flash_block_size = 64 * 2048
		self.flash_oob_size = 64
		self.flash_image_pages = 131072

	# make image
	def create(self):
		for case in switch(self.flash_page_size):
			if case(512):
				self.flash_block_size = 32 * 512
				self.flash_oob_size = 16
				break

			if case(1024):
				self.flash_block_size = 64 * 1024
				self.flash_oob_size = 32
				break

			if case(4096):
				self.flash_block_size = 64 * 4096
				self.flash_oob_size = 128
				break

		data = "\xff"
		count = 0

		data = data * (self.flash_page_size + self.flash_oob_size) # A page is 2K bytes of data + 64bytes OOB
		data_block = data * self.flash_block_size # A block is "flash_block_size" page

		fd = open(self.flash_image_name, "w+")

		total = len(data) * self.flash_image_pages / len(data_block)
		while 1:
			fd.write(data_block)
			count = count + 1
			if count == total:
				break

		fd.close()

	def put_no_oob(self, image_name, image_offset):
		data = "\xff"
		size = os.path.getsize(image_name)
		image_pages = size / self.flash_page_size
		image_page_offset = image_offset / self.flash_page_size

		if (size % self.flash_page_size) != 0:
			image_pages = image_pages + 1

		data = data * self.flash_oob_size
		fd_image = open(image_name, "rb")
		fd_flash = open(self.flash_image_name, "rw+")
		fd_flash.seek((image_page_offset * (self.flash_page_size + self.flash_oob_size)))
		i = 0
		while i < image_pages:
			read_image = fd_image.read(self.flash_page_size)
			read_image = read_image + data
			fd_flash.write(read_image)
			i = i + 1

		fd_image.close()
		fd_flash.close()

def usage():
	print 'Usage:	' + sys.argv[0] + ' <bl1> <bl2> <-o destimage> [-p page-size]\n'
	print '-o	Output to FILE'
	print '-p	Page size (default: 2048)'
	print '-h	display this help text\n'
	print 'default:'
	print 'page size	block size	oob size'
	print '512		32		16'
	print '1024		64		32'
	print '2048		64		64'
	print '4096		64		128\n'
	print 'Examples:'
	print '	' + sys.argv[0] + ' th.bin bh.bin -o flash.img'
	print '	' + sys.argv[0] + ' th.bin bh.bin -o flash.img -p 2048'

# 512K(g-bios-th)
def get_val(p):
	string = p.split('(')[0]
	K = 1024
	M = 1024 * K
	G = 1024 * M

	val = 0
	for c in string:
		if c <= '9' and c >= '0':
			val = val * 10 + int(c)
		elif c == 'G' or c == 'g':
			val = val * G
		elif c == 'M' or c == 'm':
			val = val * M
		elif c == 'K' or c == 'k':
			val = val * K

	return val

def get_parts():
	parts = {}
	try:
		sys_file = open(".sysconfig", 'r')
	except:
		print "fail to open sysconfig"
		exit(1)

	for line in sys_file:
		if re.match(r'^flash.part', line) <> None:
			elem = re.split('\s*=\s*', line.replace('\n',''))
			parts = parse_parts(elem[1])
			break;

	sys_file.close()

	return parts

def up_align(val, base):
	return ((val + base - 1) / base) * base

def parse_parts(parts_string):
	flash = {}
	flash_index = 0
	for f in parts_string.split(';'):
		parts = {}
		part_index = 0
		for p in f.split(':')[1].split(','):
			parts[part_index] = get_val(p)
			part_index = part_index + 1

		flash[flash_index] = parts;
		flash_index = flash_index + 1
	return flash[0] # fixme: default use the first flash partiontable

if __name__ == "__main__":
	argv_len = len(sys.argv)

	nand_img = nand_image()

	try:
		opts, srgs = getopt.getopt(sys.argv[1:], "o:p:h")
	except getopt.GetoptError:
		usage()
		sys.exit()

	img_names = {}
	index = 0
	opt_is_used = 0
	for opt in srgs:
		if opt_is_used == 1:
			opt_is_used = 0
			continue

		if opt == '-h':
			usage()
			sys.exit()

		if opt == '-o':
			nand_img.flash_image_name = srgs[(srgs.index(opt) + 1)]
			opt_is_used = 1
			continue

		if opt == '-p':
			nand_img.flash_page_size = int(srgs[(srgs.index(opt) + 1)])
			opt_is_used = 1
			continue

		# check img_names
		if os.path.exists(opt) == False:
			print img, "does not exists"
			usage()
			sys.exit()
		img_names[index] = opt
		index = index + 1

	if os.path.exists(nand_img.flash_image_name) == False:
		print "create image ..."
		nand_img.create()  # make image
	else:
		print nand_img.flash_image_name, "exists"
		ret = raw_input("Remaster (y or n): ")
		if ret == "y":
			print "create image ..."
			nand_img.create()  # make image

	print "image name : " + nand_img.flash_image_name
	print "page_size  : " + str(nand_img.flash_page_size)
	print "block_size : " + str(nand_img.flash_block_size)
	print "oob_size   : " + str(nand_img.flash_oob_size)

	parts = get_parts()

	offset = 0
	for index in img_names:
		print "add " + img_names[index] + " into image"
		nand_img.put_no_oob(img_names[index], offset)
		offset = offset + up_align(parts[index], nand_img.flash_block_size)
		print "put " + img_names[index] + " into flash image done!"
