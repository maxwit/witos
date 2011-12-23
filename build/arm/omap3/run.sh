#!/bin/sh

QEMUCMMOND="qemu-system-arm"
#MACHINE="beagle"
MACHINE="mw71"
NANDIMG="omap3-nand.img"

rm -vf ${NANDIMG}
./create_nand_image.py g-bios-th.bin g-bios-bh.bin g-bios-sys.bin -o ${NANDIMG}

sudo ${QEMUCMMOND} -M ${MACHINE} -mtdblock ${NANDIMG} -serial stdio -net nic -net tap
#${QEMUCMMOND} -M ${MACHINE} -mtdblock ${NANDIMG} -serial stdio -sd sd.img
