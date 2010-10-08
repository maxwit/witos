
OBJ_C = flash_core.o $(obj-y)

SUBDIRS = nand  # TODO: add nor
SUBDIR_OBJ := $(foreach n, $(SUBDIRS), $(n)/$(BUILT_IN_OBJ))

all: $(SUBDIRS) $(BUILT_IN_OBJ)

$(BUILT_IN_OBJ): $(SUBDIR_OBJ) $(OBJ_C)
	$(LD) $(LDFLAGS) -r $^ -o $@

$(SUBDIRS):
	@make -C $@ all

clean:
	@for dir in $(SUBDIRS); do \
		make -C $$dir clean; \
	 done
	@rm -vf *.o

.PHONY: $(SUBDIRS)

