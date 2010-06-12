all: $(BUILT_IN_OBJ)

$(BUILT_IN_OBJ): $(obj-y)
	$(LD) -r $(LDFLAGS) -o $@ $^

clean:
	@rm -vf *.o

