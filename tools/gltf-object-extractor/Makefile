CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c99 -pedantic -O3 -ggdb3

BUILD_DIR := build
BIN := gltf-object-extractor
SRC_FILE := $(BIN).c
OBJ_FILE := $(BUILD_DIR)/$(BIN).o

all: $(BIN)

$(BIN): $(OBJ_FILE)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_FILE): $(SRC_FILE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR) $(BIN)
