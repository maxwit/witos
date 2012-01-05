CC = gcc
CFLAGS = -Wall -g -DCONFIG_DEBUG
GB_CFLAGS = -I/maxwit/project/g-bios/include
LA_CFLAGS = -I. -include demo.h

SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)
OBJS += fs.o block.o ext2.o

all: demo

demo: $(OBJS)
	gcc $^ -o $@

demo.o: demo.c
	gcc $(CFLAGS) $(LA_CFLAGS) -c $<

bdemu_drv.o: bdemu_drv.c
	gcc $(CFLAGS) $(LA_CFLAGS) -c $<

block.o: block.c
	gcc $(CFLAGS) $(LA_CFLAGS) -c $<

fs.o: ../fs.c
	gcc $(CFLAGS) $(GB_CFLAGS) -c $<

ext2.o: ../ext2/ext2.c
	gcc $(CFLAGS) $(GB_CFLAGS) -c $<

#fat.o: ../fat/fat.c
#	gcc $(CFLAGS) -c $<

clean:
	@rm -fv *.o demo

distclean: clean
	@rm -vf $(HDRS)
