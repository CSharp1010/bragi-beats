# Determine OS
UNAME_S := $(shell uname -s)

# Compiler and flags
CC = clang
CFLAGS = -std=c99 -Wall -Wextra -g -I$(INCLUDE_DIR)

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Executable name
EXECUTABLE = $(BIN_DIR)/bragibeats

# Source files
SOURCES = $(wildcard $(SRC_DIR)/**/*.c) $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

# OS-specific flags
ifeq ($(UNAME_S),Darwin) # macOS
    CFLAGS += -I/opt/homebrew/opt/raylib/include
    LDFLAGS = -framework IOKit -framework Cocoa -framework OpenGL \
              -L/opt/homebrew/opt/raylib/lib -lraylib \
              -L/opt/homebrew/opt/fftw/lib -lfftw3f \
              -lm
else ifeq ($(UNAME_S),Linux) # Linux
    CFLAGS += -I/usr/local/include
    LDFLAGS = -L/usr/local/lib -lraylib \
              -lfftw3f \
              -lm
endif

# Default target
all: $(EXECUTABLE)

# Build the executable
$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Run the application
run: $(EXECUTABLE)
	./$(EXECUTABLE)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all run clean

