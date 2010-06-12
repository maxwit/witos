
obj-y = at91sam926x.o

obj-$(CONFIG_IRQ_SUPPORT) += at91sam926x_irq.o at91sam926x_timer.o
obj-$(CONFIG_AT91SAM9261) += at91sam9261_plat.o
obj-$(CONFIG_AT91SAM9263) += at91sam9263_plat.o

at91sam926x.o: $(TOP_DIR)/boot/arm/$(PLAT_DIR)/at91sam926x_gpio.c
	$(CC) $(CFLAGS) -c $< -o $@

at91sam926x_clock.o: $(TOP_DIR)/boot/arm/$(PLAT_DIR)/at91sam926x_clock.S
	$(CC) $(ASFLAGS) -c $< -o $@

at91sam926x_sdram.o: $(TOP_DIR)/boot/arm/$(PLAT_DIR)/at91sam926x_sdram.S
	$(CC) $(ASFLAGS) -c $< -o $@

### common rules
include $(TOP_DIR)/build/rules/general.mk

