#!/bin/sh

cp -v build/configs/arm/$1 .config

NFS_ROOT="/maxwit/lablin/rootfs"
grep "^CONFIG_NFS_ROOT" .config > /dev/null 2>&1 || \
	echo "CONFIG_NFS_ROOT=\"${NFS_ROOT}\"" >> .config

grep "^CONFIG_LOCAL_IP" .config > /dev/null 2>&1 || \
	echo "CONFIG_LOCAL_IP = MKIP(192,168,2,100)" >> .config

grep "^CONFIG_SERVER_IP" .config > /dev/null 2>&1 || \
	echo "CONFIG_SERVER_IP = MKIP(192,168,2,101)" >> .config

grep "^CONFIG_NET_MASK" .config > /dev/null 2>&1 || \
	echo "CONFIG_NET_MASK = MKIP(255,255,255,0)" >> .config

grep "^CONFIG_IMAGE_PATH" .config > /dev/null 2>&1 || \
	echo "CONFIG_IMAGE_PATH = \"/var/lib/tftpboot\"" >> .config

let "mac1 = $RANDOM & 0xff"
let "mac2 = $RANDOM & 0xff"
let "mac3 = $RANDOM & 0xff"
let "mac4 = $RANDOM & 0xff"
let "mac5 = $RANDOM & 0xff"

grep "^CONFIG_MAC_ADDR" .config > /dev/null 2>&1 || \
	echo "CONFIG_MAC_ADDR = {$mac1, $mac2, $mac3, $mac4, $mac5, 32}" >> .config
