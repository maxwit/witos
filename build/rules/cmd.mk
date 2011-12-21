SRC_C := $(wildcard $(path)/*.c)
OBJ_C ?= $(SRC_C:%.c=%.o)

SRC_S := $(wildcard $(path)/*.S)
OBJ_S ?= $(SRC_S:%.S=%.o)

obj-y = $(OBJ_C) $(OBJ_S)
obj-y := $(subst $(path)/,,$(obj-y))

#tmp_dir := $(shell mktemp -d)
tmp_dir := /tmp
dst_src = $(tmp_dir)/$^
cmd_name = $(patsubst $(path)/%.c,%,$^)

CFLAGS += -include shell.h

%.o: %.c
	@cp --parents $^ $(tmp_dir)
	@sed -i 's/\(^main.*(.*)\)/static \1/' $(dst_src)
	@sed -i 's/\(^int \{1,\}main.*(.*)\)/static \1/' $(dst_src)
	@echo >> $(dst_src)
	@grep "REGISTER_EXECUTIVE(.*, main);" $(dst_src) > /dev/null || echo "REGISTER_EXECUTIVE($(cmd_name), main);" >> $(dst_src)
	$(CC) $(CFLAGS) -c $(dst_src) -o $@
