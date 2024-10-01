# Determine OS
UNAME_S := $(shell uname -s)

# Compiler and flags
CC = clang
CFLAGS = -std=c99 -Wall -Wextra -g
SRC_FILES = main.c \
            audio_processing/audio_processing.c \
            presentation/ui_renderer.c \
            presentation/visualizers.c

EXECUTABLE = bragibeats

# OS-specific flags
ifeq ($(UNAME_S),Darwin) # macOS
    CFLAGS += -framework IOKit -framework Cocoa -framework OpenGL \
              -I/opt/homebrew/opt/raylib/include \
              -Iaudio_processing \
              -Ipresentation
    LDFLAGS = -L/opt/homebrew/opt/raylib/lib -lraylib \
              -L/opt/homebrew/opt/fftw/lib -lfftw3f \
              -lm
else ifeq ($(UNAME_S),Linux) # Linux
    CFLAGS += -I/usr/local/include \
              -Iaudio_processing \
              -Ipresentation
    LDFLAGS = -L/usr/local/lib -lraylib \
              -lfftw3f \
              -lm
endif

# Targets
all: build

build: clean $(EXECUTABLE)

$(EXECUTABLE): $(SRC_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run:
	./$(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)

.PHONY: all build run clean

