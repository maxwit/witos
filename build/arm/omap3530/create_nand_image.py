#!/usr/bin/python

import sys
import os
import getopt

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

# mkae image class
class make_image:
	def __init__ (self):
		self.flash_image_name = ""

		# default size
		self.flash_page_size   = 2048
		self.flash_oob_size    = 64
		self.flash_block_size  = 64
		self.flash_image_pages = 131072

	# make image
	def create(self):
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

	def put_no_oob(self, image_name, image_page_offset):
		data = "\xff"
		size = os.path.getsize(image_name)
		image_pages = size / self.flash_page_size

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

if __name__ == "__main__":
	argv_len = len(sys.argv)
	if argv_len < 6:
		usage()
		sys.exit()

	page_flags = 0

	# part size
	th_part_size = 0
	bh_part_size = 524288
	cfg_part_size = 2621440
	ker_part_size = 2752512
	jf2_part_size = 9043968
	yaf_part_size = 37184 * 2048
	ubi_part_size = 69952 * 2048

	mk = make_image()

	bl1 = sys.argv[1]
	bl2 = sys.argv[2]
	bl3 = sys.argv[3]

	try:
		opts, srgs = getopt.getopt(sys.argv[1:], "o:p:h")
	except getopt.GetoptError:
		usage()
		sys.exit()

	for opt in srgs:
		if opt == '-o':
			mk.flash_image_name = srgs[(srgs.index(opt) + 1)]

		if opt == '-p':
			mk.flash_page_size = int(srgs[(srgs.index(opt) + 1)])
			page_flags = 1

		if opt == '-h':
			usage()
			sys.exit()

	if os.path.exists(bl1) == False:
		print bl1, "is not exists"
		usage()
		sys.exit()

	if os.path.exists(bl2) == False:
		print bl2, "is not exists"
		usage()
		sys.exit()

	if os.path.exists(bl3) == False:
		print bl3, "is not exists"
		usage()
		sys.exit()

	if page_flags == 1:
		for case in switch(mk.flash_page_size):
			if case(512):
				mk.flash_block_size = 32
				mk.flash_oob_size = 16
				break

			if case(1024):
				mk.flash_block_size = 64
				mk.flash_oob_size = 32
				break

			if case(2048):
				mk.flash_block_size = 64
				mk.flash_oob_size = 64
				break

			if case(4096):
				mk.flash_block_size = 64
				mk.flash_oob_size = 128
				break

	print "image name : " + mk.flash_image_name
	print "page_size : " + str(mk.flash_page_size)
	print "block_size: " + str(mk.flash_block_size)
	print "oob_size  : " + str(mk.flash_oob_size)

	if os.path.exists(mk.flash_image_name) == False:
		print "create image ..."
		mk.create()  # make image
	else:
		print mk.flash_image_name, "is exists"
		ret = raw_input("Remaster (y or n): ")
		if ret == "y":
			print "create image ..."
			mk.create()  # make image

	print "add " + bl1  + " into image"
	bl1_page_offset	= th_part_size
	mk.put_no_oob(bl1, bl1_page_offset)
	print "put " + bl1 + " into flash image done!"

	print "add " + bl2  + " into image"
	bl2_page_offset = bh_part_size / mk.flash_page_size
	mk.put_no_oob(bl2, bl2_page_offset)
	print "put " + bl2 + " into flash image done!"

	print "add " + bl3  + " into image"
	bl3_page_offset = cfg_part_size / mk.flash_page_size
	mk.put_no_oob(bl3, bl3_page_offset)
	print "put " + bl3 + " into flash image done!"
