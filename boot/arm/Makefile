
# CFLAGS += $(PLAT_OPT)
# ASFLAGS += $(PLAT_OPT)

obj-y = head.o cpu.o cpuid.o

SUBDIRS = $(PLAT_DIR)
SUBDIR_OBJ := $(foreach n, $(SUBDIRS), $(n)/$(BUILT_IN_OBJ))

ALL_SUBDIRS = $(shell find -maxdepth 1 -type d -name "[a-zA-Z0-9_]*")

all: $(BUILT_IN_OBJ)

$(BUILT_IN_OBJ): $(obj-y) $(SUBDIR_OBJ)
	$(LD) $(LDFLAGS) -r $^ -o $(BUILT_IN_OBJ)

$(SUBDIR_OBJ): $(SUBDIRS)/Makefile $(SUBDIRS)

# fixme
$(SUBDIRS)/Makefile:
	@cp rules.mk $(SUBDIRS)/Makefile

$(SUBDIRS): 
	@make -C $@ all

clean:
	@for dir in $(ALL_SUBDIRS); do \
		make -C $$dir clean; \
	 done	
	@rm -vf *.o

.PHONY: $(SUBDIRS) head.S
