SRC_C := $(wildcard *.c)
SRC_C := $(filter-out __temp_gapp_%.c, $(SRC_C))
OBJ_C ?= $(SRC_C:%.c=%.o)

SRC_S := $(wildcard *.S)
OBJ_S ?= $(SRC_S:%.S=%.o)

all: $(BUILT_IN_OBJ) 

$(BUILT_IN_OBJ) : $(OBJ_C) $(OBJ_S)
	$(LD) $(LDFLAGS) -r $^ -o $@

%.o: %.c
	@cp $^ __temp_gapp_$^
	@sed -i 's/\(^main.*(.*)\)/static \1/' __temp_gapp_$^
	@sed -i 's/\(^int \{1,\}main.*(.*)\)/static \1/' __temp_gapp_$^
	@echo >>  __temp_gapp_$^
	@grep "include.*g-bios.h" __temp_gapp_$^ > /dev/null || echo "#include <g-bios.h>" >> __temp_gapp_$^
	@echo "INSTALL_APPLICATION($*, main);" >> __temp_gapp_$^
	$(CC) $(CFLAGS) -c __temp_gapp_$^ -o $@

$(OBJ_S): $(SRC_S)

clean:
	@rm -vf *.o
	@rm -f __temp_gapp_*.c
