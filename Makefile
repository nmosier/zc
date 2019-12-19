.PHONY: all
all: test

CFLAGS = -std=c++2a -Iast

test: test.cpp
	c++ $(CFLAGS) -o $@ $^

