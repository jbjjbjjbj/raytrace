CC=gcc

# Enable debugging
CFLAGS=-ggdb

# We need math
LDFLAGS=-lm

# Output and build options
BINARY=raytrace
BUILD_DIR=build

# Input files
c_files=$(wildcard *.c)
OBJ=$(patsubst %.c, $(BUILD_DIR)/%.o, $(c_files))

# Build rules
# $@ is the %.o file and $^ is the %.c file
$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c -o $@ $^ $(CFLAGS)

# $@ becomes left part thus linked
$(BINARY): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: run show clean

# This will also generate the image
run: $(BINARY)
	./$(BINARY) | convert - test.png

show: run
	xdg-open test.png

clean:
	rm -f $(OBJ) $(BINARY)
	rmdir $(BUILD_DIR)
