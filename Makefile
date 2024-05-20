# Define build directory
BUILD_DIR := build

# Default target
all: build

# Make build directory and Execute cmake and build
build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..
	@cmake --build $(BUILD_DIR)

# Clean target
clean:
	@rm -rf $(BUILD_DIR)

# Rebuild
rebuild: clean all

.PHONY: all build clean rebuild