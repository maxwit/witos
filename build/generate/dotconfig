#!/bin/sh

cp -v build/configs/arm/$1 .config

NFS_ROOT="${HOME}/maxwit/rootfs"
grep "^CONFIG_NFS_ROOT=" .config > /dev/null 2>&1 || \
	echo "CONFIG_NFS_ROOT=\"${NFS_ROOT}\"" >> .config

# sed -i "s:\(.*\)CONFIG_NFS_ROOT=.*:export CONFIG_NFS_ROOT=\"${NFS_ROOT}\":" .config
