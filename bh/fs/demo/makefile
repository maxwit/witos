CFLAGS = -Wall -g -I../../../include #-D__arm__
CC = gcc

#SRCS = $(wildcard *.c)
#OBJS = $(SRCS:%.c=%.o)

COMMON_OBJS = block.o drive.o foo_drv.o

all: fat_demo

ext_demo: $(COMMON_OBJS) ext_demo.o
	gcc $^ -o $@

fat_demo: $(COMMON_OBJS) #fat_demo.o

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	@rm -fv *.o fat_demo ext_demo
