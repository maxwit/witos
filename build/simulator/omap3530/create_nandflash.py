#!/usr/bin/python

import sys
import os
import getopt

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

# default Nand flash partitions
#0    0x00000000 - 0x00080000  512K      g-bios-th  "g-bios"
#1    0x00080000 - 0x00280000  2M        g-bios-bh  "g-bios"
#2    0x00280000 - 0x002A0000  128K      sysconfig  "g-bios"
#3    0x002A0000 - 0x005A0000  3M        linux      "linux"
#4    0x005A0000 - 0x008A0000  3M        ramdisk    "ramdisk"
#5    0x008A0000 - 0x048A0000  64M       jffs2      "rootfs"
#6    0x048A0000 - 0x088A0000  64M       yaffs2     "data_1"
#7    0x088A0000 - 0x10000000  119M384K  ubifs      "data_2"

# make image
def create(flash_block_size, flash_oob_size, flash_image_name, flash_page_size, flash_image_pages):
	data = "\xff"
	count = 0

	data = data * (flash_page_size + flash_oob_size) # A page is 2K bytes of data + 64bytes OOB
	data_block = data * flash_block_size # A block is "flash_block_size" page

	fd = open(flash_image_name, "w+");

	total = len(data) * flash_image_pages / len(data_block)
	while 1:
		fd.write(data_block)
		count = count + 1
		if count == total:
			break

	fd.close()

def put_no_oob(image_name, image_page_offset, flash_page_size):
	size = os.path.getsize(image_name)
	image_pages = size / flash_page_size

	if (size % flash_page_size) != 0:
		image_pages = image_pages + 1

	i = 0
	while i < image_pages:
		out_offset = image_page_offset + i
		in_offset = i

		cmd = "dd if=" + image_name + " of=" + flash_image_name + " conv=notrunc count=1 obs=" + str(flash_page_size+flash_oob_size) + " ibs=" + str(flash_page_size) + " seek=" + str(out_offset) + " skip=" + str(in_offset)

		os.system(cmd) #fixme
		i = i + 1

def usage():
	print 'Usage:	' + sys.argv[0] + ' <-i image> <-o destimage>'
	print '	[-p page-size] [-b block-size] [-f oob-size]\n'
	print '-b	Block size (default: 64 page)'
	print '-f	OOB size (default: 64)'
	print '-i	Input image FILE (g-bios-th.bin, g-bios-bh.bin, zImage, rootfs)'
	print '	rootfs:	yaffs2, jffs2, ubifs'
	print '-o	Output to FILE'
	print '-p	Page size (default: 2048)'
	print '-h	display this help text\n'
	print 'default:'
	print 'page size	block size	oob size'
	print '512		32		16'
	print '1024		64		32'
	print '2048		64		64'
	print '4069		64		128\n'
	print 'Examples:'
	print '	' + sys.argv[0] + ' -i g-bios-th.bin -o flash.img'
	print '	' + sys.argv[0] + ' -i zImage -o flash.img'
	print '	' + sys.argv[0] + ' -i ubifs -o flash.img'
	print '	' + sys.argv[0] + ' -i g-bios-th -o flash.img -p 2048'
	print '	' + sys.argv[0] + ' -i g-bios-th -o flash.img -p 2048 -b 64 -f 64'

if __name__ == "__main__":
	argv_len = len(sys.argv)
	if argv_len < 5:
		usage()
		sys.exit()

	# default size
	flash_page_size	  = 2048
	flash_oob_size    = 64
	flash_block_size  = 64
	flash_image_pages = 131072
	oob_size   = 64
	block_size = 64

	# part size
	th_part_size = 0
	bh_part_size = 524288
	ker_part_size = 2752512
	jf2_part_size = 9043968
	yaf_part_size = 37184 * 2048
	ubi_part_size = 69952 * 2048

	oob_flags   = 0
	block_flags = 0
	page_flags  = 0

	try:
		opts, srgs = getopt.getopt(sys.argv[1:], "b:f:i:o:p:h")
	except getopt.GetoptError:
		usage()
		sys.exit()

	for opt, arg in opts:
		if opt == '-b':
			flash_block_size = int(arg)
			block_flags = 1

		if opt == '-f':
			flash_oob_size = int(arg)
			oob_flags = 1

		if opt == '-i':
			image = arg

		if opt == '-o':
			flash_image_name = arg

		if opt == '-p':
			flash_page_size = int(arg)
			page_flags = 1

		if opt == '-h':
			usage()
			sys.exit()

	if os.path.exists(flash_image_name):
		print flash_image_name, "is exists"
	else:
		if page_flags == 1:
			for case in switch(flash_page_size):
				if case(512):
					block_size = 32
					oob_size = 16
					break

				if case(1024):
					block_size = 64
					oob_size = 32
					break

				if case(2048):
					block_size = 64
					oob_size = 64
					break

				if case(4096):
					block_size = 64
					oob_size = 128
					break

		if block_flags == 1:
			if flash_block_size != block_size:
				usage()
				sys.exit()
		else:
			flash_block_size = block_size

		if oob_size == 1:
			if flash_oob_size != oob_size:
				usage()
				sys.exit()
		else:
			flash_oob_size = oob_size

		if block_flags == 1 & oob_flags == 1:
			if page_flags != 1:
				usage()
				sys.exit()

		print "page_size : " + str(flash_page_size)
		print "block_size: " + str(flash_block_size)
		print "oob_size  : " + str(flash_oob_size)
		print "create image ..."
		create(flash_block_size, flash_oob_size, flash_image_name, flash_page_size, flash_image_pages)

	for case in switch(image):
		if case("g-bios-th.bin"):
			print "bl1 image name: g-bios-th.bin"
			bl1_page_offset	= th_part_size
			put_no_oob("g-bios-th.bin", bl1_page_offset, flash_page_size)
			print "put bl1 into flash image done!"
			break

		if case("g-bios-bh.bin"):
			print "bl2 image name: g-bios-th.bin"
			bl2_page_offset = bh_part_size / flash_page_size
			put_no_oob("g-bios-bh.bin", bl2_page_offset, flash_page_size)
			print "put BL2 into flash image done!"
			break

		if case("zImage"):
			print "Linux kernel image name: zImage"
			kernel_page_offset = ker_part_size / flash_page_size
			put_no_oob("zImage", kernel_page_offset, flash_page_size)
			print "put Linux kernel into flash image done!"
			break

		if case("jffs2"):
			print "rootfs image name: jffs2"
			jffs2_page_offset = jf2_part_size / flash_page_size
			put_no_oob("jffs2", jffs2_page_offset, flash_page_size)
			print "put jffs2 into flash image done!"
			break

		if case("yaffs2"):
			print "rootfs image name: yaffs2"
			yaffs2_page_offset = yaf_part_size / flash_page_size
			put_no_oob("yaffs2", yaffs2_page_offset, flash_page_size)
			print "put yaffs2 into flash image done!"
			break

		if case("ubifs"):
			print "rootfs image name: ubifs"
			ubifs_page_offset = ubi_part_size / flash_page_size
			put_no_oob("ubifs", ubifs_page_offset, flash_page_size)
			print "put ubifs into flash image done!"
			break
		else:
			usage()
			sys.exit()
