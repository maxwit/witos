#pragma once

enum s3cxx_cpu_type {
	TYPE_S3C2410,
	TYPE_S3C2412,
	TYPE_S3C2440,
	TYPE_S3C6410,
};

#ifdef CONFIG_S3C2410
#define NAND_CTRL_BASE  0x4e000000
#define NF_CONF               0x00
#define NF_CMMD               0x04
#define NF_ADDR               0x08
#define NF_DATA               0x0c
#define NF_STAT               0x10
#define NF_ECC                0x14
#endif

#ifdef CONFIG_S3C2412
#define NAND_CTRL_BASE  0x4e000000
#define NF_CONF               0x00
#endif

#ifdef CONFIG_S3C2440
#define NAND_CTRL_BASE  0x4e000000
#define NF_CONF               0x00
#define NF_CONT               0x04
#define NF_CMMD               0x08
#define NF_ADDR               0x0c
#define NF_DATA               0x10
#define NF_STAT               0x20
#define NF_ECC0               0x2c
#define NF_SBLK               0x38
#define NF_EBLK               0x3c
#endif

#ifdef CONFIG_S3C6410
#define NAND_CTRL_BASE 0x70200000
#define NF_CONF        0x00
#define NF_CONT        0x04
#define NF_CMMD        0x08
#define NF_ADDR        0x0c
#define NF_DATA        0x10
#define NF_STAT        0x28
#endif
