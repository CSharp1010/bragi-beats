# Determine OS
UNAME_S := $(shell uname -s)

# Compiler and flags
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra
SRC_FILES =     infrastructure/main.c                   business/audio_processing.c \
                presentation/ui_rendering.c              presentation/visualizers.c \
                persistence/library_init.c     infrastructure/playback.c \
                infrastructure/file_processing.c
EXECUTABLE = bragibeats

# OS-specific flags
ifeq ($(UNAME_S),Darwin) # macOS
    CFLAGS += -framework IOKit -framework Cocoa -framework OpenGL \
              -I/opt/homebrew/opt/raylib/include -Iraylib \
              -I/opt/homebrew/opt/mysql/include/mysql -Imysql \
              -I/opt/homebrew/opt/openssl@3/include -Iopenssl \
              -Ibusiness -Ipersistence -Ipresentation -Iinfrastructure
    LDFLAGS = -L/opt/homebrew/opt/raylib/lib -lraylib \
              -L/opt/homebrew/opt/mysql/lib -lmysqlclient \
              -L/opt/homebrew/opt/zstd/lib -lzstd \
              -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto -lresolv -lm \
              -L/opt/homebrew/opt/fftw/lib -lfftw3f
else ifeq ($(UNAME_S),Linux) # Linux
    CFLAGS += -I/usr/local/include -Iraylib -Imysql -Iopenssl -Ibusiness -Ipersistence -Ipresentation -Iinfrastructure
    LDFLAGS = -L/usr/local/lib -lraylib -lmysqlclient -lzstd -lssl -lcrypto -lm -lfftw3f
endif

# Targets
all: install-dependencies build

install-dependencies:
ifeq ($(UNAME_S),Darwin)
	@brew install raylib mysql openssl zstd fftw || true
else ifeq ($(UNAME_S),Linux)
	@sudo apt-get update && sudo apt-get install -y libraylib-dev libmysqlclient-dev libssl-dev libzstd-dev libfftw3-dev || true
endif

build: clean $(EXECUTABLE)

$(EXECUTABLE): $(SRC_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

run:
	./$(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)

.PHONY: all install-dependencies build run clean
