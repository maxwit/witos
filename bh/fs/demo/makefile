CFLAGS = -Wall -g
CC = gcc

SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)

demo: $(OBJS) ext.o
	gcc $^ -o $@

ext.o: ../ext2/ext.c

clean:
	rm -fv demo $(OBJS)
