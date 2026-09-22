CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror

TEST_BIN = sp1_unit_tests
TEST_SOURCES = src/main.c src/test_gain.c \
  ../../src/core/audio.c ../../src/core/controls.c

.PHONY: all run clean

all: $(TEST_BIN)

$(TEST_BIN): $(TEST_SOURCES) ../../src/sp1_api.h
	$(CC) $(CFLAGS) -DSP1_HOST_TEST -I../../src -o $@ $(TEST_SOURCES)

run: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(TEST_BIN)
