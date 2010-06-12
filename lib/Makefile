
OBJ_C = 

SUBDIRS = stdlib string flash net misc

SUBDIR_OBJ := $(foreach n, $(SUBDIRS), $(n)/$(BUILT_IN_OBJ))

all: $(SUBDIRS) $(BUILT_IN_OBJ)

$(SUBDIRS): 
	@make -C $@ all

$(BUILT_IN_OBJ): $(OBJ_C) $(SUBDIR_OBJ)
	$(LD) $(LDFLAGS) -r $^ -o $@

clean:
	@for dir in $(SUBDIRS); do \
		make -C $$dir clean; \
	 done	
	@rm -vf *.o

.PHONY: $(SUBDIRS)

