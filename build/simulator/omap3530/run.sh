#!/bin/sh

QEMUCMMOND="qemu-system-arm"
MACHINE="beagle"
NANDIMG="omap3-nand.img"

./bb_nandflash.sh g-bios-th.bin ${NANDIMG}
./bb_nandflash.sh g-bios-bh.bin ${NANDIMG}

${QEMUCMMOND} -M ${MACHINE} -mtdblock ${NANDIMG} -serial stdio -net nic -net tap
#${QEMUCMMOND} -M ${MACHINE} -mtdblock ${NANDIMG} -serial stdio
