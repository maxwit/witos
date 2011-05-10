#!/bin/sh
#
# Usage:
#   $0 .config autoconf.h

DOT_CONFIG=$1
AUTO_CONFIG=$2

ARCH=`grep -w "CONFIG_ARCH" $1 | sed 's/.*=//'`

# fixme!!!
if `grep CONFIG_S3C6410 .config > /dev/null 2>&1`; then
	PLAT_DIR=s3c6410
elif `grep CONFIG_S3C2410 .config > /dev/null 2>&1`; then
	PLAT_DIR=s3c24x0
elif `grep CONFIG_S3C2440 .config > /dev/null 2>&1`; then
	PLAT_DIR=s3c24x0
elif `grep CONFIG_AT91SAM9261 .config > /dev/null 2>&1`; then
	PLAT_DIR=at91sam926x
elif `grep CONFIG_AT91SAM9263 .config > /dev/null 2>&1`; then
	PLAT_DIR=at91sam926x
elif `grep CONFIG_OMAP3530 .config > /dev/null 2>&1`; then
	PLAT_DIR=omap3530
else
	echo "Platform not specified!"
	exit 1
fi

echo "#pragma once" > ${AUTO_CONFIG}
echo >> ${AUTO_CONFIG}

#sed -e 's:^#://:' -e 's/^\(CONFIG_.*\)=[\s]*y/#define \1/' -e 's/^\(CONFIG_.*\)=\(.*\)/#define \1 \2/' ${DOT_CONFIG} >> ${AUTO_CONFIG}
sed -e '/^#/d' -e 's/^\(CONFIG_.*\)=[\s]*y/#define \1/' -e 's/^\(CONFIG_.*\)=\(.*\)/#define \1 \2/' ${DOT_CONFIG} >> ${AUTO_CONFIG}

echo >> ${AUTO_CONFIG}

printf "#include <%s/cpu.h>\n" ${ARCH} >> ${AUTO_CONFIG}
printf "#include <%s/%s.h>\n" ${ARCH} ${PLAT_DIR} >> ${AUTO_CONFIG}
