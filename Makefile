BUILD_DIR=build

include $(N64_INST)/include/n64.mk
include $(N64_INST)/include/t3d.mk

N64_CFLAGS += -std=gnu++17 -ftrivial-auto-var-init=uninitialized -fno-exceptions -Os 
N64_CXXFLAGS += -ftrivial-auto-var-init=uninitialized -fno-exceptions -Os  -I src
MKSPRITE_FLAGS = -c 3
PROJECT_NAME=n64brew_gamejam

src += $(wildcard src/*.cpp)
src += $(wildcard src/*/*.cpp)

# handle our asset
# assets_png = $(wildcard assets/*.png)
# assets_gltf = $(wildcard assets/*.glb)
# assets_conv = $(addprefix filesystem/,$(notdir $(assets_png:%.png=%.sprite))) \
# 			  $(addprefix filesystem/,$(notdir $(assets_gltf:%.glb=%.t3dm)))
assets_conv += $(addprefix filesystem/,$(notdir $(wildcard assets/tracks/*.bin)))

all: $(PROJECT_NAME).z64
	
filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o filesystem "$<"

filesystem/%.t3dm: assets/%.glb
	@mkdir -p $(dir $@)
	@echo "    [T3D-MODEL] $@"
	$(T3D_GLTF_TO_3D) "$<" $@
	$(N64_BINDIR)/mkasset -c 3 -o filesystem $@

filesystem/%.bin: assets/tracks/%.bin
	@mkdir -p $(dir $@)
	@echo "    [TRACK] $@"
	$(N64_BINDIR)/mkasset -c 1 -o filesystem $<


$(BUILD_DIR)/$(PROJECT_NAME).dfs: $(assets_conv)
$(BUILD_DIR)/$(PROJECT_NAME).elf: $(src:%.cpp=$(BUILD_DIR)/%.o)

$(PROJECT_NAME).z64: N64_ROM_TITLE="Gamejam"
$(PROJECT_NAME).z64: $(BUILD_DIR)/$(PROJECT_NAME).dfs

clean:
	rm -rf $(BUILD_DIR) *.z64
	rm -rf filesystem

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
