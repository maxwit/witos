
# CFLAGS += $(PLAT_OPT)
# ASFLAGS += $(PLAT_OPT)

OBJ_C = arm_heap.o exception.o
OBJ_S = head.o 

SUBDIRS = $(PLAT_DIR) lib
ALL_SUBDIRS = $(shell find -maxdepth 1 -type d -name "[a-zA-Z0-9_]*")

SUBDIR_OBJ := $(foreach n, $(SUBDIRS), $(n)/$(BUILT_IN_OBJ))

all: $(SUBDIRS) $(BUILT_IN_OBJ)

$(SUBDIRS): 
	@make -C $@ all

$(BUILT_IN_OBJ): $(OBJ_S) $(OBJ_C) $(SUBDIR_OBJ)
	$(LD) -r $(LDFLAGS) -o $@ $^

clean:
	@for dir in $(ALL_SUBDIRS); do \
		make -C $$dir clean; \
	 done	
	@rm -vf *.o

.PHONY: $(SUBDIRS)

