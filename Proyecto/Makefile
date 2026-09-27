CC = gcc
CFLAGS = -std=gnu11 -O2 -Wall -Wextra -MMD -MP -Iinclude
LDLIBS = -lm

SRCS = $(filter-out src/main.c, $(wildcard src/*.c))
OBJS = $(SRCS:src/%.c=build/%.o)
TESTS = $(patsubst tests/%.c,build/%,$(wildcard tests/test_*.c))

all: build/placer

build/placer: build/main.o $(OBJS)
	$(CC) $^ -o $@ $(LDLIBS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/test_%: tests/test_%.c $(OBJS) | build
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDLIBS)

test: $(TESTS)
	@for t in $(TESTS); do ./$$t || exit 1; done

build:
	mkdir -p build

clean:
	rm -rf build

.PHONY: all test clean

-include build/*.d
