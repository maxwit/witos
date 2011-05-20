CFLAGS = -Wall -g
CC = gcc

OBJS = ext2.o block.o demo.o
SRCS = $(OBJS:%.c=%.o)

demo_ext2: $(OBJS)
	gcc $^ -o $@

clean:
	rm -fv demo_ext2 $(OBJS)
