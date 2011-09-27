CFLAGS = -Wall -g -I../../../include -DCONFIG_DEBUG -DCONFIG_HOST_DEMO
CC = gcc

SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)

all: demo

demo: $(OBJS)
	gcc $^ -o $@

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	@rm -fv *.o demo
