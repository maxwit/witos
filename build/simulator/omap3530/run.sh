#!/bin/sh

QEMUCMMOND="qemu-system-arm"
MACHINE="beagle"
#MACHINE="mw71"
NANDIMG="omap3-nand.img"

rm -vf $NANDIMG
./bb_nandflash.sh g-bios-th.bin ${NANDIMG}
./bb_nandflash.sh g-bios-bh.bin ${NANDIMG}
./bb_nandflash.sh zImage ${NANDIMG}
./bb_nandflash.sh rootfs ${NANDIMG}

#sudo ${QEMUCMMOND} -M ${MACHINE} -mtdblock ${NANDIMG} -serial stdio -net nic -net tap -sd sd.img
${QEMUCMMOND} -M ${MACHINE} -mtdblock ${NANDIMG} -serial stdio
