.PHONY: all
all: test

CFLAGS = -std=c++17 -Iast

test: test.c
	c++ $(CFLAGS) -o $@ $^

