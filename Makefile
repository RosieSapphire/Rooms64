BUILD_DIR := build
FS_DIR    := filesystem
include $(N64_INST)/include/n64.mk

ROM_NAME_STR := "Rooms64"
ROM_NAME     := rooms

DFS_FILE  := $(BUILD_DIR)/$(ROM_NAME).dfs
ELF_FILE  := $(BUILD_DIR)/$(ROM_NAME).elf
Z64_FILE  := $(ROM_NAME).z64

SRC_DIRS := src src/engine src/game
C_FILES  := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
O_FILES  := $(C_FILES:%.c=$(BUILD_DIR)/%.o)

INC_DIRS   := include
INC_FLAGS  := $(INC_DIRS:%=-I%)
N64_CFLAGS += -Wall -Wextra -Werror -pedantic $(INC_FLAGS)

MDL_CONV_DIR := tools/modelconv
MDL_CONV     := $(MDL_CONV_DIR)/build/modelconv.elf
DAT_FIND     := tools/roomdatfinder/roomdatfinder

ROOM_MDLS := room0 room1 room2 room3 room4 room5

ASSETS_WAV := $(wildcard assets/*.wav)
ASSETS_PNG := $(wildcard assets/*.png)
ASSETS_TTF := $(wildcard assets/*.ttf)
ASSETS_MDL := $(wildcard $(MDL_CONV_DIR)/res/glb/*.glb)
ASSETS_DAT := $(ROOM_MDLS:%=$(MDL_CONV_DIR)/res/glb/%.glb)

ASSETS_CONV := $(ASSETS_WAV:assets/%.wav=$(FS_DIR)/%.wav64) \
	       $(ASSETS_PNG:assets/%.png=$(FS_DIR)/%.sprite) \
	       $(ASSETS_TTF:assets/%.ttf=$(FS_DIR)/%.font64) \
	       $(ASSETS_MDL:$(MDL_CONV_DIR)/res/glb/%.glb=$(FS_DIR)/%.mdl) \
	       $(ASSETS_DAT:$(MDL_CONV_DIR)/res/glb/%.glb=$(FS_DIR)/%.dat)

AUDIOCONV_FLAGS := --ym-compress true
MKSPRITE_FLAGS  := -c 1

.PHONY: all clean

all: $(Z64_FILE)

$(FS_DIR)/%.wav64: assets/%.wav
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	@$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) $<

$(FS_DIR)/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	@$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o $(dir $@) "$<"

$(FS_DIR)/debug_font.font64: assets/debug_font.ttf
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	@$(N64_MKFONT) --size 8 -o $(dir $@) "$<"

$(FS_DIR)/door_num_font.font64: assets/door_num_font.ttf
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	@$(N64_MKFONT) --size 36 -o $(dir $@) "$<"

$(FS_DIR)/%.mdl: tools/modelconv/res/glb/%.glb
	@mkdir -p $(dir $@)
	@echo "    [MDL] $@"
	$(MDL_CONV) $< $(dir $@)
	# TODO: Maybe use mkasset for this

$(FS_DIR)/%.dat: tools/modelconv/res/glb/%.glb
	@mkdir -p $(dir $@)
	@echo "    [DAT] $@"
	$(DAT_FIND) $<
	# TODO: Maybe use mkasset for this
	cp $(patsubst %.glb,%.dat,$<) $@

$(DFS_FILE): $(ASSETS_CONV)
$(ELF_FILE): $(O_FILES)

$(Z64_FILE): N64_ROM_TITLE=$(ROM_NAME_STR)
$(Z64_FILE): $(DFS_FILE)
# $(Z64_FILE): N64_ED64ROMCONFIGFLAGS=-w eeprom4k

clean:
	rm -rf $(BUILD_DIR) $(Z64_FILE) compile_commands.json .cache/ $(FS_DIR)/
