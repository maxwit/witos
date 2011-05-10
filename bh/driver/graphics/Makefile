obj-y = display_core.o video_mode.o

obj-$(CONFIG_BOOTUP_LOGO) += logo.o

obj-$(CONFIG_GRAPHICS_AT91)   += at91_display.o
obj-$(CONFIG_GRAPHICS_S3C24X) += s3c24x_display.o
obj-$(CONFIG_GRAPHICS_S3C64X) += s3c64x_display.o
obj-$(CONFIG_GRAPHICS_OMAP3)  += omap3_display.o

$(path)/logo.c: $(TOP_DIR)/doc/pic/g-bios_logo.jpg
	@xxd -i $< > $@
	@sed -i 's/unsigned\ .*\[\]\ =/unsigned\ char\ _gbios_jpg\[\]\ =/' $@
	@sed -i 's/unsigned\ .*len\ =/unsigned\ int\ _gbios_jpg_len\ =/' $@
