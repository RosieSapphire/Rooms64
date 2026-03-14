BUILD_DIR := build
DEBUG     ?=

PROG := $(BUILD_DIR)/modelconv.elf

CC := clang-20
#CFLAGS := -Wall -Wextra -Werror -pedantic -ansi
WARN_IGNORE := -Wno-reserved-macro-identifier -Wno-reserved-identifier \
	       -Wno-format-nonliteral -Wno-unsafe-buffer-usage
CFLAGS := -Weverything -Werror -pedantic -std=c99 \
	  -I../../vendor/rose_petal $(WARN_IGNORE)
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
