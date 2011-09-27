CFLAGS = -Wall -g -I../../../include -DCONFIG_DEBUG -DCONFIG_HOST_DEMO
CC = gcc

SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)
OBJS += drive.o block.o ext2.o fat.o fs.o

all: demo

demo: $(OBJS)
	gcc $^ -o $@

clean:
	@rm -fv *.o demo

block.o: ../../driver/block/block.c
	gcc $(CFLAGS) -c $<

drive.o: ../../driver/block/drive.c
	gcc $(CFLAGS) -c $<

ext2.o: ../ext/ext2.c
	gcc $(CFLAGS) -c $<

#ext2.h: ../ext/ext2.h

fat.o: ../fat/fat.c
	gcc $(CFLAGS) -c $<

#fat.h: ../fat/fat.h

fs.o: ../fs.c
	gcc $(CFLAGS) -c $<
