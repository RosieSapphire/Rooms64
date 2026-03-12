BUILD_DIR := build
DEBUG     ?= 1

PROG := $(BUILD_DIR)/modelconv.elf

CC := gcc
#CFLAGS := -Wall -Wextra -Werror -pedantic -ansi
CFLAGS := --no-warnings
ifdef DEBUG
	CFLAGS += -O0 -ggdb3 -fsanitize=address,undefined,leak,null -D_DEBUG
else
	CFLAGS += -O3 -g0 -DNDEBUG
endif
LNK_FLAGS := -lassimp -lm

all: $(PROG)
	@echo "Built successfully! \"$<\""

$(PROG): $(BUILD_DIR)/modelconv.o
	@echo "    [LD] $@"
	@$(CC) $(CFLAGS) -o $@ $< $(LNK_FLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "    [CC] $<"
	@$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(BUILD_DIR) $(PROG) compile_commands.json .cache/
