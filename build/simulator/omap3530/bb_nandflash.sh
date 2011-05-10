#!/bin/sh
# Copyright (C) 2008 yajin (yajin@vm-kernel.org)
#
# Put the BL1,BL2,kernel and rootfs into nand flash image.
# 
# bb_nandflash.sh  <BL1-image>		<nandflashimage>  BL1
# bb_nandflash.sh  <BL2-image>		<nandflashimage>  BL2	
# bb_nandflash.sh  <kernel-image>   <nandflashimage>  kernel
# bb_nandflash.sh  <rootfs>			<nandflashimage>  rootfs

# We assume that rootfs image has oob info 
# while BL1 BL2 and kernel image does not have oob info.


if [ ! -r "$1" ]; then
	echo "Usage: $0 <image> <destimage>"
	exit -1
fi
if [ 2 -ne "$#" ]; then
	echo "Usage: $0 <image> <destimage>"
	exit -1
fi

# Nand flash partitions
# 0x00000000-0x00080000 : "BL1" 
# 0x00080000-0x00260000 : "BL2" 
# 0x00260000-0x00280000 : "BL2 config"
# 0x00280000-0x00680000 : "Kernel" 
# 0x00680000-0x10000000 : "File System" 

flash_page_size=2048
flash_oob_size=64
flash_image_pages=131072

bl1_page_offset=0
bl2_page_offset=256
kernel_page_offset=1280
rootfs_page_offset=3328

flash_image_name=$2
bl1_image_name=$1
bl2_image_name=$1
kernel_image_name=$1
rootfs_image_name=$1

echo "flash image name:"$flash_image_name

create_img()
{
	#beagle board's NAND flash is 2G bit(256M bytes)
	if [ ! -e "${1}" ]; then
		echo "${1}" " does not exist.Create it!"
		echo -en \\0377\\0377\\0377\\0377\\0377\\0377\\0377\\0377 > .8b
		cat .8b .8b > .16b 
		cat .16b .16b >.32b
		cat .32b .32b >.64b  #OOB is 64 bytes
		cat .64b .64b .64b .64b .64b .64b .64b .64b > .512b
		cat .512b .512b .512b .512b .64b>.page  # A page is 2K bytes of data + 64bytes OOB
		cat .page .page .page .page .page .page .page .page >.8page
		cat .8page .8page .8page .8page .8page .8page .8page .8page >.block  # a block = 64 pages
		cat .block .block .block .block .block .block .block .block >.8block
		cat .8block .8block .8block .8block .8block .8block .8block .8block >.64block
		cat .64block .64block .64block .64block .64block .64block .64block .64block >.512block
		cat .512block .512block .512block .512block >"${1}"
		rm -rf .8b .16b .32b .64b .512b .page .8page .64sec .block .8block .64block .512block 
	fi
}

put_no_oob() 
{
	#echo $1
	#echo $2
	image_name=$1
	image_page_offset=$2
	image_len=`du -shb $image_name |awk '{print $1}'`
	image_pages=$[$image_len/2048]

	if [ 0 -ne $[$image_len%$flash_page_size] ]; then
		image_pages=$[$image_pages+1]
	fi

	#echo $image_len
	#echo $image_pages
	i=0
	while  [ $i -lt $image_pages  ]
	do
  	#echo $i
  	out_offset=$[$image_page_offset+$i]
  	in_offset=$i
  	#echo "out_offset:"$out_offset
  	#echo "in_offset:"$in_offset
  	dd if=$image_name of=$flash_image_name conv=notrunc count=1 obs=$[$flash_page_size+$flash_oob_size] ibs=$flash_page_size  seek=$out_offset skip=$in_offset  
  	i=$[$i + 1]
	done	
}

put_bl1()
{
	echo "bl1 image name:"$bl1_image_name
	put_no_oob $1 $bl1_page_offset
	echo "put bl1 into flash image done!"
}

put_bl2()
{
	echo "bl2 image name:"$bl2_image_name
	put_no_oob $1 $bl2_page_offset
	echo "put BL2 into flash image done!"
}

put_kernel()
{
	echo "Linux kernel image name:"$kernel_image_name
	put_no_oob $1 $kernel_page_offset
	echo "put Linux kernel into flash image done!"
}

put_rootfs()
{
	echo "rootfs image name:"$rootfs_image_name
	put_no_oob $1 $rootfs_page_offset
	echo "put rootfs into flash image done!"
}

create_img ${2}

case ${1} in
	g-bios-th.bin)
		put_bl1 ${1}
		;;
	g-bios-bh.bin)
		put_bl2 ${1}
		;;
	kernel)
		put_kernel ${1}
		;;
	rootfs)
		put_rootfs ${1}
		;;
	*)
		echo "Unknown partition $1"
		exit -1
esac
