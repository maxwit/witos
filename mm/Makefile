

SUBDIRS = heap

SUB_SUBDIR_OBJ := $(foreach n, $(SUBDIRS), $(n)/$(BUILT_IN_OBJ))

all: $(SUBDIRS) $(BUILT_IN_OBJ)

$(BUILT_IN_OBJ): $(SUB_SUBDIR_OBJ)
	$(LD) $(LDFLAGS) -r $^ -o $@

$(SUBDIRS):
	@make -C $@ all

clean:
	@rm -f *.o
	@for dir in $(SUBDIRS); do \
		make -C $$dir clean; \
	done

.PHONY: $(SUBDIRS)
