
CFLAGS += -DCONFIG_GTH
ASFLAGS += -DCONFIG_GTH

SUBDIRS = $(ARCH) common
SUBDIR_OBJ := $(foreach n, $(SUBDIRS), $(n)/$(BUILT_IN_OBJ))

all: $(SUBDIRS) $(IMAGE_OUT_DIR)/g-bios-th.elf

$(IMAGE_OUT_DIR)/g-bios-th.elf: $(SUBDIR_OBJ)
	$(LD) $(LDFLAGS) -T $(ARCH)/g-bios-th.lds -Ttext $(CONFIG_GTH_START_MEM) $^ -o $@
	$(OBJCOPY) -O binary -S $@ $(@:.elf=.bin)
	$(OBJDUMP) -D $@ > $(@:.elf=.dis)

$(SUBDIRS):
	@make -C $@ all

clean:
	@for dir in $(SUBDIRS); do \
		make -C $$dir clean; \
	 done	
	@rm -vf *.o g-bios-th.*

.PHONY: $(SUBDIRS)
