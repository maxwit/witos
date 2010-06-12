
OBJ_C =  malloc.o

all: $(BUILT_IN_OBJ)

$(BUILT_IN_OBJ): $(OBJ_C)
	$(LD) -r $(LDFLAGS) -o $@ $^

clean:
	@rm -vf *.o

