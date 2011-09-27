CFLAGS = -Wall -g -I../../../include -DCONFIG_DEBUG -DCONFIG_HOST_DEMO
CC = gcc

#SRCS = $(wildcard *.c)
#OBJS = $(SRCS:%.c=%.o)

COMMON_OBJS = block.o drive.o foo_drv.o fs.o

all: demo

demo: $(COMMON_OBJS) fat.o ext2.o demo.o
	gcc $^ -o $@

ext_demo: $(COMMON_OBJS) ext2.o ext2_demo.o
	gcc $^ -o $@

fat_demo: $(COMMON_OBJS) fat.o fat_demo.o

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	@rm -fv *.o fat_demo ext2_demo demo
