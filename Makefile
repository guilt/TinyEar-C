CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -O2 -Iinclude
LDFLAGS ?= -lm

.PHONY: all test clean

all: ear test_host

ear: src/ear.c src/main.c include/ear.h
	$(CC) $(CFLAGS) src/ear.c src/main.c -o ear $(LDFLAGS)

test_host: src/ear.c tests/test_host.c include/ear.h
	$(CC) $(CFLAGS) src/ear.c tests/test_host.c -o test_host $(LDFLAGS)

test: test_host
	./test_host

clean:
	rm -rf ear test_host
