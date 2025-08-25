# Game Config
DEBUG_ENABLED := 1
NOCLIPPING_ENABLED := 1
RENDER_AABB := 1
MAX_ROOMS_ACTIVE_AT_ONCE := 3
TOTAL_ROOM_COUNT := 1000
COMPRESS_LEVEL := 0
MODEL_SCALE := 100
TICKRATE := 30
USE_INTERPOLATION := 1
AUDIO_SAMPLERATE := 48000
AUDIO_BUFFER_COUNT := 1

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
	      -DMAX_ROOMS_ACTIVE_AT_ONCE=$(MAX_ROOMS_ACTIVE_AT_ONCE) \
	      -DTOTAL_ROOM_COUNT=$(TOTAL_ROOM_COUNT) \
	      -DCOMPRESS_LEVEL=$(COMPRESS_LEVEL) \
	      -DMODEL_SCALE=$(MODEL_SCALE) \
	      -DTICKRATE=$(TICKRATE) \
	      -DAUDIO_SAMPLERATE=$(AUDIO_SAMPLERATE) \
	      -DAUDIO_BUFFER_COUNT=$(AUDIO_BUFFER_COUNT)

ifeq ($(DEBUG_ENABLED),1)
	N64_CFLAGS += -DDEBUG
endif

ifeq ($(NOCLIPPING_ENABLED),1)
	N64_CFLAGS += -DPLAYER_NOCLIP
endif

ifeq ($(RENDER_AABB),1)
	N64_CFLAGS += -DRENDER_AABB
endif

ifeq ($(USE_INTERPOLATION),1)
	N64_CFLAGS += -DUSE_INTERPOLATION
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

AUDIOCONV_FLAGS := --wav-compress $(COMPRESS_LEVEL) --wav-mono \
		   --wav-resample $(AUDIO_SAMPLERATE)
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
	grep -i --color=always "todo" $^
	grep -i --color=always "fixme" $^

-include $(wildcard $(BUILD_DIR)/*.d)
