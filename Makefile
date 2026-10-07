# ==============================================================================
# ESP32-S3 Baremetal / Command-Line Makefile
# ==============================================================================

PORT ?= COM3
BAUD ?= 115200
TARGET ?= esp32s3
BUILD_DIR ?= build
PYTHON ?= python

.PHONY: all build flash monitor clean erase help

# Default target: build the firmware
all: build

# Build project using cmake / idf
build:
	@echo "==> Building project for $(TARGET)..."
	@if exist $(BUILD_DIR) ( \
		cmake --build $(BUILD_DIR) \
	) else ( \
		cmake -B $(BUILD_DIR) -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=$(IDF_PATH)/tools/cmake/toolchain-esp32s3.cmake . && \
		cmake --build $(BUILD_DIR) \
	)

# Flash binary to ESP32-S3 via esptool
flash:
	@echo "==> Flashing binary to $(PORT)..."
	$(PYTHON) -m esptool --chip $(TARGET) -p $(PORT) -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0 $(BUILD_DIR)/bootloader/bootloader.bin 0x8000 $(BUILD_DIR)/partition_table/partition-table.bin 0x10000 $(BUILD_DIR)/ESP32-S3-DevKitC-1-v1.1-All-Protocol-Drivers.bin

# Open Serial Monitor in Terminal (uses pyserial)
monitor:
	@echo "==> Opening Serial Terminal on $(PORT) at $(BAUD) baud..."
	@echo "==> Press [Ctrl] + [C] or [Ctrl] + []] to exit."
	$(PYTHON) -m serial.tools.miniterm $(PORT) $(BAUD)

# Flash and immediately start monitor
flash-monitor: flash monitor

# Erase ESP32 Flash
erase:
	@echo "==> Erasing flash on $(PORT)..."
	$(PYTHON) -m esptool --chip $(TARGET) -p $(PORT) erase_flash

# Clean build artifacts
clean:
	@echo "==> Cleaning build directory..."
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	@echo "Clean completed."

# Help menu
help:
	@echo -----------------------------------------------------------
	@echo  ESP32-S3 Makefile Commands
	@echo -----------------------------------------------------------
	@echo  make               : Build the project
	@echo  make flash         : Flash firmware to $(PORT)
	@echo  make monitor       : Open Serial Terminal monitor on $(PORT)
	@echo  make flash-monitor : Flash firmware and immediately monitor
	@echo  make clean         : Remove $(BUILD_DIR) directory
	@echo  make erase         : Erase entire ESP32 flash chip
	@echo -----------------------------------------------------------
	@echo  Override port: make flash PORT=COM3
	@echo -----------------------------------------------------------
