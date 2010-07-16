
obj-$(CONFIG_IRQ_SUPPORT) += s3c6410_irq.o s3c6410_timer.o
obj-$(CONFIG_S3C6410) += s3c6410_plat.o

s3c6410_clock.o: $(TOP_DIR)/boot/arm/$(PLAT_DIR)/s3c6410_clock.c
	$(CC) $(CFLAGS) -c $< -o $@

# common rules
include $(TOP_DIR)/build/rules/general.mk
