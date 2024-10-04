# test/Makefile

# Compiler and flags
CC = clang
CFLAGS = -std=c99 -Wall -Wextra -g -DUNIT_TESTING -DFFT_SIZE=16384 -I../audio_processing -I.. \
		 -I/opt/homebrew/opt/raylib/include

# Source files
TEST_FILES = test_audioProcessing.c unity.c
SRC_FILES = ../audio_processing/audio_processing.c

# Executable name
TEST_EXECUTABLE = test_audioProcessing

# Libraries
LIBS = -L/opt/homebrew/opt/raylib/lib -lraylib -lm

# Targets
all: $(TEST_EXECUTABLE)

$(TEST_EXECUTABLE): $(TEST_FILES) $(SRC_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	./$@

clean:
	rm -f $(TEST_EXECUTABLE)

.PHONY: all clean

