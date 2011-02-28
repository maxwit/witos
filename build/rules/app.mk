SRC_C := $(wildcard $(path)/*.c)
OBJ_C ?= $(SRC_C:%.c=%.o)

SRC_S := $(wildcard $(path)/*.S)
OBJ_S ?= $(SRC_S:%.S=%.o)

obj-y = $(OBJ_C) $(OBJ_S)
obj-y := $(subst $(path)/,,$(obj-y))

#tmp_dir := $(shell mktemp -d)
tmp_dir := /tmp
dst_src = $(tmp_dir)/$^

app_name = $(patsubst $(path)/%.c,%,$^)

%.o: %.c
	@cp --parents $^ $(tmp_dir)
	@sed -i 's/\(^main.*(.*)\)/static \1/' $(dst_src)
	@sed -i 's/\(^int \{1,\}main.*(.*)\)/static \1/' $(dst_src)
	@echo >>  $(dst_src)
	@sed -i 's/\(^char\ app_option\[\]\[CMD_OPTION_LEN.*\)/static \1/' $(dst_src)
	@sed -i 's/\(^app_usr_opt_match.*(.*)\)/static \1/' $(dst_src)
	@sed -i 's/\(^int \{1,\}app_usr_opt_match.*(.*)\)/static \1/' $(dst_src)
	@sed -i 's/\(^app_usr_cmd_match.*(.*)\)/static \1/' $(dst_src)
	@sed -i 's/\(^int \{1,\}app_usr_cmd_match.*(.*)\)/static \1/' $(dst_src)
	@grep "app_usr_opt_match" $(dst_src) > /dev/null || echo "INSTALL_APPLICATION($(app_name), main, app_option, app_usr_cmd_match, NULL);" >> $(dst_src)
	@grep "INSTALL_APPLICATION(.*, main, app_option, app_usr_cmd_match, NULL);" $(dst_src) > /dev/null || echo "INSTALL_APPLICATION($(app_name), main, app_option, app_usr_cmd_match, app_usr_opt_match);" >> $(dst_src)
	@grep "^static char app_option" $(dst_src) > /dev/null || sed -i 's/app_option/NULL/' $(dst_src)
	@grep "^static.*app_usr_cmd_match" $(dst_src) > /dev/null || sed -i 's/app_usr_cmd_match/NULL/' $(dst_src)
	$(CC) $(CFLAGS) -c $(dst_src) -o $@

#$(OBJ_S): $(SRC_S)
