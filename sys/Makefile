
SYSCONFIG = g-bios-sc
CROSS_COMPILE = arm-maxwit-linux-gnueabi-

all: $(SYSCONFIG).inf

$(SYSCONFIG).inf: $(SYSCONFIG).elf
	$(CROSS_COMPILE)objcopy -O binary -S $< $@

$(SYSCONFIG).elf: $(SYSCONFIG).o
	$(CROSS_COMPILE)ld -m armelf_linux_eabi -T $(SYSCONFIG).lds -Ttext 0x0 -o $@ $^

$(SYSCONFIG).o: $(SYSCONFIG).S
	$(CROSS_COMPILE)gcc -c $<

clean:
	@rm -vf *.o *.elf *.inf
