-include .config

include $(img)/Makefile

dir-y:= $(foreach n, $(dir-y), $(img)/$(n))

subdir-objs := $(foreach n, $(dir-y), $(n)/$(builtin-obj))

include build/rules/common.mk

ifeq "$(img)" "th"
G_START_MEM ?= $(CONFIG_GTH_START_MEM)
else
G_START_MEM ?= $(CONFIG_GBH_START_MEM)
endif

#$(img): include/autoconf.h $(dir-y) $(img)/g-bios-$(img).bin $(img)/g-bios-$(img).dis
$(img): include/autoconf.h $(dir-y) $(img)/g-bios-$(img).bin
	@echo

include/autoconf.h: .config
	@build/generate_autoconf $< $@

$(img)/g-bios-$(img).bin: $(img)/g-bios-$(img).elf
	$(OBJCOPY) -O binary -S $< $@

$(img)/g-bios-$(img).dis: $(img)/g-bios-$(img).elf
	$(OBJDUMP) -D $< > $@

$(img)/g-bios-$(img).elf: $(subdir-objs)
	$(LD) $(LDFLAGS) -T $(img)/$(CONFIG_ARCH)/g-bios-$(img).lds -Ttext $(G_START_MEM) $^ -o $@

$(dir-y):
	@make $(obj_build)$@

.PHONY: $(dir-y)

clean:
	@for dir in $(dir-y); do \
		make $(obj_build)$$dir clean; \
	 done
	@rm -vf g-bios-$(img).*
