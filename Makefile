# compiler and flags
CC=gcc
CFLAGS=-Itest -std=c99 -Wall -Wextra -framework IOKit -framework Cocoa -framework OpenGL \
-I/opt/homebrew/opt/raylib/include -Iraylib \
-I/opt/homebrew/opt/mysql/include/mysql -Imysql \
-I/opt/homebrew/opt/openssl@3/include -Iopenssl \
-Ibusiness -Ipersistence -Ipresentation -Iinfrastructure

LDFLAGS=-L/opt/homebrew/opt/raylib/lib -lraylib \
-L/opt/homebrew/opt/mysql/lib -lmysqlclient \
-L/opt/homebrew/opt/zstd/lib -lzstd \
-L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto -lresolv -lm

# source files
SRC_FILES=business/audioProcessing.c
UNITY_FILES=test/unity.c
TEST_FILES=test/test_audioProcessing.c

# builds to output
EXECUTABLE=run_tests

# Build and run tests
all: clean $(EXECUTABLE) run

$(EXECUTABLE): $(SRC_FILES) $(UNITY_FILES) $(TEST_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

run:
	./$(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)

.PHONY: all clean run

