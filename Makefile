# Minimal wrapper: configure CMake (Ninja) once, then build with Ninja.
# Targets: build (default), configure, reconfigure, clean, distclean
#
# Usage:
#   make                # = make build (Release)
#   make build
#   make build BUILD_TYPE=Debug
#   make reconfigure    # re-run cmake with current BUILD_TYPE
#   make clean          # ninja tool clean (no CMake regen)
#   make distclean      # remove build directory
#
# Notes:
#   - Expects PICO_SDK_PATH in your shell env (or your pico_sdk_import.cmake handles it).
#   - Artifacts end up in ./build/ (e.g., steppydemo.uf2 from your CMakeLists).

BUILD       ?= build
CMAKE       ?= cmake
NINJA       ?= ninja
BUILD_TYPE  ?= Release

# Default target
.PHONY: all
all: build

.PHONY: configure
configure:
	@mkdir -p "$(BUILD)"
	@if [ ! -f "$(BUILD)/build.ninja" ]; then \
	  "$(CMAKE)" -S . -B "$(BUILD)" -G Ninja -DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" -DCMAKE_SUPPRESS_REGENERATION=ON; \
	else \
	  echo "configure: $(BUILD)/build.ninja already exists (use 'make reconfigure' to force)."; \
	fi

.PHONY: build
build:
	@test -f "$(BUILD)/build.ninja" || $(MAKE) configure BUILD_TYPE="$(BUILD_TYPE)"
	"$(NINJA)" -C "$(BUILD)"

.PHONY: reconfigure
reconfigure:
	@mkdir -p "$(BUILD)"
	@rm -f "$(BUILD)/CMakeCache.txt"
	"$(CMAKE)" -S . -B "$(BUILD)" -G Ninja -DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" -DCMAKE_SUPPRESS_REGENERATION=ON

.PHONY: clean
clean:
	@if [ -f "$(BUILD)/build.ninja" ]; then \
	  "$(NINJA)" -C "$(BUILD)" -t clean; \
	else \
	  echo "clean: nothing to do (no $(BUILD)/build.ninja)."; \
	fi

.PHONY: distclean
distclean:
	@rm -rf "$(BUILD)"

.PHONY: flash
flash: build
	@echo "Looking for BOOTSEL volume..."
	@if [ -d "/Volumes/RP2350" ]; then \
	  cp "$(BUILD)/steppydemo.uf2" /Volumes/RP2350/; \
	  echo "Flashed to RP2350."; \
	elif [ -d "/Volumes/RPI-RP2" ]; then \
	  cp "$(BUILD)/steppydemo.uf2" /Volumes/RPI-RP2/; \
	  echo "Flashed to RPI-RP2."; \
	else \
	  echo "Error: No BOOTSEL volume found. Hold BOOTSEL while plugging in USB."; \
	  exit 1; \
	fi

.PHONY: serial
serial:
	@echo "Looking for Pico serial port..."
	@PICO_PORT=$$(ls /dev/cu.usbmodem* 2>/dev/null | head -1); \
	if [ -z "$$PICO_PORT" ]; then \
		echo "Error: No Pico found. Make sure the device is connected and running firmware."; \
		echo "Available serial ports:"; \
		ls /dev/cu.* 2>/dev/null || echo "  (none found)"; \
		exit 1; \
	else \
		echo "Connecting to $$PICO_PORT at 115200 baud..."; \
		echo "Use Ctrl+A, X to exit minicom"; \
		minicom -D $$PICO_PORT -b 115200; \
	fi

.PHONY: size
size: build
	@arm-none-eabi-size "$(BUILD)/steppydemo.elf"

