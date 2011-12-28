-include .config

MAJOR_VER = 2
MINOR_VER = 5

TOP_DIR := $(shell pwd)
IMG_DIR := $(CONFIG_IMAGE_PATH)

CROSS_COMPILE = $(CONFIG_CROSS_COMPILE:"%"=%)

AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -ffreestanding -nostdinc -nostdlib -fno-builtin -I$(TOP_DIR)/include -include g-bios.h -DGBIOS_VER_MAJOR=$(MAJOR_VER) -DGBIOS_VER_MINOR=$(MINOR_VER) -D__G_BIOS__ -D__LITTLE_ENDIAN -O2 -Wall -Werror-implicit-function-declaration -mno-thumb-interwork -march=$(CONFIG_ARCH_VER) -mabi=aapcs-linux -mpoke-function-name

#ifeq ($(CONFIG_DEBUG),y)
#	CFLAGS += -DCONFIG_DEBUG
#endif

# fxime: to add "-mtune=xxx, -mfloat-abi=xxx"

ASFLAGS = $(CFLAGS) -D__ASSEMBLY__

LDFLAGS = -m armelf_linux_eabi

builtin-obj = built-in.o

MAKEFLAGS = --no-print-directory

export AS CC LD OBJDUMP OBJCOPY ASFLAGS CFLAGS LDFLAGS MAKEFLAGS
export builtin-obj TOP_DIR

# fixme
DEFCONFIG_PATH = build/configs/arm
DEFCONFIG_LIST = $(shell cd $(DEFCONFIG_PATH) && ls *_defconfig)

include build/rules/common.mk

dir-y := th bh

all: $(dir-y)

$(dir-y): include/autoconf.h
	@make $(img_build)$@

include/autoconf.h: .config
	@build/generate/autoconf.py $< $@
	@sed -i -e '/CONFIG_CROSS_COMPILE/d' -e '/CONFIG_ARCH_VER\>/d'  $@
	@sed -i '/^$$/d' $@

# fixme
$(DEFCONFIG_LIST):
	@echo "configure for board \"$(@:%_defconfig=%)\""
	@grep -w "^sysG" $(DEFCONFIG_PATH)/$(@:%_defconfig=%_sysconfig) || echo sysG > .sysconfig && echo >> .sysconfig
	@cat $(DEFCONFIG_PATH)/$(@:%_defconfig=%_sysconfig) >> .sysconfig
	@./build/generate/defconfig.py $@
	@echo

install: th/g-bios-th.bin bh/g-bios-bh.bin
	@mkdir -p $(IMG_DIR)
	@for fn in $^; do \
		cp -v $$fn $(IMG_DIR); \
	done
	@echo

clean:
	@for dir in $(dir-y); do \
		make $(img_build)$$dir clean; \
		rm -vf $$dir/g-bios-$$dir.*; \
	 done
	@echo

distclean: clean
	@rm -vf .config include/autoconf.h
	@echo

.PHONY: $(dir-y)
