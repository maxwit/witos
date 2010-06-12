
obj-y := main.o stdio.o uart.o

obj-$(CONFIG_NAND_SUPPORT) += nand.o
obj-$(CONFIG_KERMIT_SUPPORT) += kermit.o
obj-$(CONFIG_YMODEM_SUPPORT) += ymodem.o

all: $(BUILT_IN_OBJ)

$(BUILT_IN_OBJ): $(obj-y)
	$(LD) -r $(LDFLAGS) -o $@ $^

uart.o: $(TOP_DIR)/driver/uart/uart.c
	$(CC) $(CFLAGS) -c $< -o $@

kermit.o: $(TOP_DIR)/driver/uart/kermit.c
	$(CC) $(CFLAGS) -c $< -o $@

ymodem.o: $(TOP_DIR)/driver/uart/ymodem.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -vf *.o

