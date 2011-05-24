-include .config

MAJOR_VER = 2
MINOR_VER = 5

TOP_DIR := $(shell pwd)

CROSS_COMPILE = $(CONFIG_CROSS_COMPILE:"%"=%)

AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy

ASFLAGS = $(CFLAGS) -D__ASSEMBLY__

# fxime: to add "-mtune=xxx"
CFLAGS = -ffreestanding -nostdinc -nostdlib -fno-builtin -I$(TOP_DIR)/include -include g-bios.h -DGBIOS_VER_MAJOR=$(MAJOR_VER) -DGBIOS_VER_MINOR=$(MINOR_VER) -mno-thumb-interwork -march=$(CONFIG_ARCH_VER) -mabi=aapcs-linux -O2 -mpoke-function-name -DGBH_START_BLK=$(CONFIG_GBH_START_BLK) -DGBH_START_MEM=$(CONFIG_GBH_START_MEM) -Wall -Werror-implicit-function-declaration

#ifeq ($(CONFIG_DEBUG),y)
#	CFLAGS += -DCONFIG_DEBUG
#endif

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
	@./build/generate/dotconfig.sh $@
	@echo

defconfig:
	@./build/generate/defconfig.py

#####echo "******************"
#####echo "*   .config      *"
#####echo "******************"

install:
	@mkdir -p $(CONFIG_IMAGE_PATH)
	@for fn in $(wildcard [tb]h/g-bios-*.bin); do \
		cp -v $$fn $(CONFIG_IMAGE_PATH); \
	done
	@echo
	@ls -l $(CONFIG_IMAGE_PATH)/g-bios-[tb]h.bin
	@echo

clean:
	@for dir in $(dir-y); do \
		make $(img_build)$$dir clean; \
		rm -vf $$dir/g-bios-$$dir.*; \
	 done

distclean: clean
	@rm -vf .config include/autoconf.h

.PHONY: $(dir-y)
