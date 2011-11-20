-include .config

MAJOR_VER = 2
MINOR_VER = 5

TOP_DIR := $(shell pwd)
IMG_DIR := $(CONFIG_IMAGE_PATH)/boot

CROSS_COMPILE = $(CONFIG_CROSS_COMPILE:"%"=%)

AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy

# fxime: to add "-mtune=xxx"
CFLAGS = -ffreestanding -nostdinc -nostdlib -fno-builtin -I$(TOP_DIR)/include -include g-bios.h -DGBIOS_VER_MAJOR=$(MAJOR_VER) -DGBIOS_VER_MINOR=$(MINOR_VER) -mno-thumb-interwork -march=$(CONFIG_ARCH_VER) -mabi=aapcs-linux -O2 -mpoke-function-name -Wall -Werror-implicit-function-declaration -D__G_BIOS__ -D__LITTLE_ENDIAN

#ifeq ($(CONFIG_DEBUG),y)
#	CFLAGS += -DCONFIG_DEBUG
#endif

ASFLAGS = $(CFLAGS) -D__ASSEMBLY__

LDFLAGS = -m armelf_linux_eabi

builtin-obj = built-in.o

MAKEFLAGS = --no-print-directory

export AS CC LD OBJDUMP OBJCOPY ASFLAGS CFLAGS LDFLAGS MAKEFLAGS
export builtin-obj TOP_DIR

include build/rules/common.mk

dir-y := th bh

all: $(dir-y)

$(dir-y): include/autoconf.h
	@make $(img_build)$@

include/autoconf.h: .config
	@build/generate/autoconf.py $< $@
	@sed -i '/CONFIG_CROSS_COMPILE/d' $@
	@sed -i '/^$$/d' $@

# fixme
%_defconfig:
	@echo
	@#./build/generate/dotconfig.sh $@
	@./build/generate/defconfig.py $@
	@echo

#####echo "******************"
#####echo "*   .config      *"
#####echo "******************"

install:
	@mkdir -p $(IMG_DIR)
	@for fn in $(wildcard [tb]h/g-bios-*.bin); do \
		cp -v $$fn $(IMG_DIR); \
	done
	@echo

clean:
	@for dir in $(dir-y); do \
		make $(img_build)$$dir clean; \
		rm -vf $$dir/g-bios-$$dir.*; \
	 done

distclean: clean
	@rm -vf .config include/autoconf.h

.PHONY: $(dir-y)
