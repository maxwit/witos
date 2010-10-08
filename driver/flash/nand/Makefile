
obj-y = nand_core.o nand_bbt.o nand_ecc.o nand_ids.o

obj-$(CONFIG_S3C2410) += s3cxx_nand.o
obj-$(CONFIG_S3C2440) += s3cxx_nand.o
obj-$(CONFIG_S3C6410) += s3cxx_nand.o
obj-$(CONFIG_AT91SAM9261) += at91sam9261_nand.o
obj-$(CONFIG_AT91SAM9263) += at91sam9263_nand.o

include $(TOP_DIR)/build/rules/general.mk
