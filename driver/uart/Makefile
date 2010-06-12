
obj-y = uart.o kermit.o ymodem.o

obj-$(CONFIG_AT91SAM9261) += at91sam926x_uart.o
obj-$(CONFIG_AT91SAM9263) += at91sam926x_uart.o
obj-$(CONFIG_S3C2410) += s3c24x0_uart.o
obj-$(CONFIG_S3C2440) += s3c24x0_uart.o

s3c24x0_uart.o: $(TOP_DIR)/boot/arm/$(PLAT_DIR)/s3c24x0_uart.c
	$(CC) $(CFLAGS) -c $< -o $@

at91sam926x_uart.o: $(TOP_DIR)/boot/arm/$(PLAT_DIR)/at91sam926x_uart.c
	$(CC) $(CFLAGS) -c $< -o $@

include $(TOP_DIR)/build/rules/general.mk
