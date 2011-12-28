dir-y += font
# dir-$(CONFIG_FONT) += font
dir-$(CONFIG_BOOTUP_LOGO) += jpeg
obj-$(CONFIG_BOOTUP_LOGO) += logo.o

$(path)/logo.c: $(TOP_DIR)/resource/g-bios-logo.jpg
	@xxd -i $< > $@
	@sed -i 's/unsigned\ .*\[\]\ =/unsigned\ char\ _gbios_jpg\[\]\ =/' $@
	@sed -i 's/unsigned\ .*len\ =/unsigned\ int\ _gbios_jpg_len\ =/' $@
