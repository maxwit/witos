CFLAGS = -Wall -I. -include init.h -g -DCONFIG_DEBUG
CC = gcc

SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)
OBJS += fs.o block.o ext2.o

HDRS =  list.h fs

all: $(HDRS) demo

demo: $(OBJS)
	gcc $^ -o $@

clean:
	@rm -fv *.o demo

ext2.o: ../ext2/ext2.c
	gcc $(CFLAGS) -c $<

#fat.o: ../fat/fat.c
#	gcc $(CFLAGS) -c $<

fs.o: ../fs.c
	gcc $(CFLAGS) -c $<

$(HDRS):
	@ln -sv ../../../include/$@ $@

distclean: clean
	@rm -vf $(HDRS)
