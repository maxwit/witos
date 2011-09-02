CFLAGS = -Wall -g
CC = gcc

SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)

demo: $(OBJS)
	gcc $^ -o $@

clean:
	rm -fv demo $(OBJS)
