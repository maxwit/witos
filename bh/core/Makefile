obj-y = init.o irq.o timer.o task.o sysconf.o device.o confdata.o

$(path)/confdata.c: $(TOP_DIR)/.sysconfig
	@xxd -i $< > $@
	@sed -i 's/\(.*\) _.*sysconfig\(.*\)/\1 _g_sysconfig\2/' $@
	@sed -i 's/unsigned char \(.*\)/unsigned char __SYSCONF__ \1/' $@

PHONY += main.c
