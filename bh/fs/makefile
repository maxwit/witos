CFLAGS = -Wall
CC = gcc

OBJS = ext2.o main.o block.o
SRCS = $(OBJS:%.c=%.o)

demo_ext2: $(OBJS)
	gcc $^ -o $@

clean:
	rm -fv demo_ext2 $(OBJS)
