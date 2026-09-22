CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror
LDFLAGS ?=

TEST_BIN = sp1_host_tests
TEST_SOURCES = test_main.c test_reverb.c

.PHONY: all run wav clean

all: $(TEST_BIN)

$(TEST_BIN): $(TEST_SOURCES) ../dsp/reverb/sp1_dsp_reverb.h test.h
	$(CC) $(CFLAGS) -I. -o $@ $(TEST_SOURCES) $(LDFLAGS)

run: $(TEST_BIN)
	./$(TEST_BIN)

wav: $(TEST_BIN)
	./$(TEST_BIN) --wav reverb_test.wav

clean:
	rm -f $(TEST_BIN) reverb_test.wav
