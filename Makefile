CXXFLAGS += -std=c++2a -I ast/
HEADERS = ast.hpp $(wildcard ast/*.hpp)

.PHONY: all
all: test c.tab.o

.PHONY: bison
bison: c.tab.cpp

c.tab.cpp: c.ypp $(HEADERS)
	bison $<

.PHONY: flex
flex: c.l
	flex --c++ c.l

%.o: %.cpp
	c++ -c $(CXXFLAGS) -o $@ $^

test: test.o $(HEADERS)
	c++ $(OFLAGS) -o $@ $<

