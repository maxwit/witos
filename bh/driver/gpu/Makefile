obj-y = gpu_core.o vmode_db.o

obj-$(CONFIG_BOOTUP_LOGO) += logo.o

$(path)/logo.c: $(TOP_DIR)/doc/pic/g-bios_logo.jpg
	@xxd -i $< > $@
	@sed -i 's/unsigned\ .*\[\]\ =/unsigned\ char\ _gbios_jpg\[\]\ =/' $@
	@sed -i 's/unsigned\ .*len\ =/unsigned\ int\ _gbios_jpg_len\ =/' $@
