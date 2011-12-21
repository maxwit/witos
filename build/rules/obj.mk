all:

include .config

path := $(patsubst %/,%,$(path))

cmd_dir := bh/app/command

# check if $(path) is the sub directory of app
# ifeq ($(patsubst $(cmd_dir)%,$(cmd_dir),$(path)),$(cmd_dir))
ifeq ($(path),$(cmd_dir))
include build/rules/cmd.mk
else
include $(path)/Makefile

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.S
	$(CC) $(ASFLAGS) -o $@ -c $<
endif

include build/rules/common.mk

PHONY := $(foreach n, $(PHONY), $(path)/$(n))

obj-y := $(foreach n, $(obj-y), $(path)/$(n))
subdir-obj := $(foreach n, $(dir-y), $(path)/$(n)/built-in.o)

builtin-obj := $(path)/built-in.o

all: $(dir-y) $(builtin-obj)

$(builtin-obj): $(obj-y) $(subdir-obj)
	$(LD) $(LDFLAGS) -r $^ -o $@

PHONY += $(dir-y)
$(dir-y):
	@make $(obj_build)$(path)/$@

clean: .config
	@for dir in $(dir-y); do \
		make $(obj_build)$(path)/$$dir clean; \
	 done
	@rm -vf $(path)/*.o

.PHONY: $(PHONY) FORCE
