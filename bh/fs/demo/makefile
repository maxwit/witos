CFLAGS = -Wall -g -I. -DCONFIG_DEBUG
CC = gcc

SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)
OBJS += fs.o block.o ext2.o fat.o

all: demo

demo: $(OBJS)
	gcc $^ -o $@

clean:
	@rm -fv *.o demo

ext2.o: ../ext/ext2.c
	gcc $(CFLAGS) -c $<

#fat.o: ../fat/fat.c
#	gcc $(CFLAGS) -c $<

fs.o: ../fs.c
	gcc $(CFLAGS) -c $<
