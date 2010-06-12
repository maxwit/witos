
SUBDIRS = flash memory net serial sysconf boot shell misc

SUBDIR_OBJ := $(foreach n, $(SUBDIRS), $(n)/$(BUILT_IN_OBJ))

all: $(SUBDIRS) $(BUILT_IN_OBJ)

$(BUILT_IN_OBJ): $(SUBDIR_OBJ)
	$(LD) $(LDFLAGS) -r $^ -o $@

$(SUBDIRS): 
	@make -C $@ all

clean:
	@for dir in $(shell ls); do \
		test -d $$dir && make -C $$dir clean; \
	 done	
	@rm -f *.o

.PHONY: $(SUBDIRS)
