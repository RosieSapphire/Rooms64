# Game Config
NOCLIPPING_ENABLED := 1
RENDER_HITBOXES := 1

DEBUG_ENABLED := 1
COMPRESS_LEVEL := 3
MODEL_SCALE := 100
BUILD_DIR := build

include $(N64_INST)/include/n64.mk
include $(T3D_INST)/t3d.mk

INC_DIRS := include include/engine include/game
INC_FLAGS := $(INC_DIRS:%=-I%)
SRC_DIRS := src src/engine src/game
H_FILES := $(foreach dir,$(INC_DIRS),$(wildcard $(dir)/*.h))
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
O_FILES := $(C_FILES:%.c=$(BUILD_DIR)/%.o)

TARGET := rooms-remake
ROM := $(TARGET).z64
ELF := $(BUILD_DIR)/$(TARGET).elf
DFS := $(BUILD_DIR)/$(TARGET).dfs
N64_CFLAGS += -Wall -Wextra -Werror -Ofast $(INC_FLAGS) \
	      -DCOMPRESS_LEVEL=$(COMPRESS_LEVEL) -DMODEL_SCALE=$(MODEL_SCALE)
ifeq ($(DEBUG_ENABLED),1)
	N64_CFLAGS += -DDEBUG
endif

ifeq ($(NOCLIPPING_ENABLED),1)
	N64_CFLAGS += -DPLAYER_NOCLIP
endif

ifeq ($(RENDER_HITBOXES),1)
	N64_CFLAGS += -DAABB_RENDER
endif

ASSETS_PNG := $(wildcard assets/*.png)
ASSETS_WAV := $(wildcard assets/*.wav)
ASSETS_GLTF := $(wildcard assets/*.gltf)
ASSETS_ROOM := $(wildcard assets/room*.gltf)
ASSETS_CONV := $(ASSETS_PNG:assets/%.png=filesystem/%.sprite) \
	       $(ASSETS_WAV:assets/%.wav=filesystem/%.wav64) \
	       $(ASSETS_GLTF:assets/%.gltf=filesystem/%.t3dm) \
	       $(ASSETS_ROOM:assets/%.gltf=filesystem/%.room)

final: $(ROM)
$(ROM): N64_ROM_TITLE="Rooms 64"
$(ROM): $(DFS) 
$(DFS): $(ASSETS_CONV)
$(ELF): $(O_FILES)

AUDIOCONV_FLAGS := --wav-compress $(COMPRESS_LEVEL)
MKSPRITE_FLAGS := --compress $(COMPRESS_LEVEL)
MKASSET_FLAGS := --compress $(COMPRESS_LEVEL)

GLTF_TO_ROOM := tools/gltf-object-extractor/gltf-object-extractor

filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o $(dir $@) "$<"

filesystem/%.wav64: assets/%.wav
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

filesystem/%.t3dm: assets/%.gltf
	@mkdir -p $(dir $@)
	@echo "    [T3D-MODEL] $@"
	$(T3D_GLTF_TO_3D) "$<" $@ --base-scale=$(MODEL_SCALE)
	$(N64_BINDIR)/mkasset $(MKASSET_FLAGS) -o $(dir $@) $@

filesystem/%.room: assets/%.gltf
	@mkdir -p $(dir $@)
	@echo "    [ROOM] $@"
	$(GLTF_TO_ROOM) $< $@
	$(N64_BINDIR)/mkasset $(MKASSET_FLAGS) -o $(dir $@) $@

.PHONY: clean todo

clean:
	rm -rf $(ROM) $(BUILD_DIR) filesystem

todo: $(H_FILES) $(C_FILES)
	grep -i "todo" $^
	grep -i "fixme" $^

-include $(wildcard $(BUILD_DIR)/*.d)
