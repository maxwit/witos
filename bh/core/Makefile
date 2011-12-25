obj-y = init.o delay.o irq.o timer.o task.o sysconf.o device.o confdata.o

$(path)/confdata.c: $(TOP_DIR)/.sysconfig
	@xxd -i $< > $@
	@sed -i 's/unsigned char \(.*\)/unsigned char __SYSCONF__ \1/' $@

PHONY += main.c
