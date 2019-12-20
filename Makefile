CXXFLAGS += -std=c++2a -I ast/
HEADERS = ast.hpp $(wildcard ast/*.hpp)

BISON_SRC = c.tab.cpp
FLEX_SRC = lex.yy.cc

BISON_OBJ = $(BISON_SRC:.cpp=.o)
FLEX_OBJ = $(FLEX_SRC:.cc=.o)

.PHONY: all
all: test c.tab.o flex

.PHONY: bison
bison: $(BISON_OBJ)

$(BISON_SRC): c.ypp $(HEADERS)
	bison $<

.PHONY: flex
flex: $(FLEX_OBJ)

$(FLEX_SRC): c.l $(HEADERS)
	flex --c++ $<

%.o: %.cpp
	c++ -c $(CXXFLAGS) -o $@ $^

%.o: %.cc
	c++ -c $(CXXFLAGS) -o $@ $^

test: test.o $(HEADERS)
	c++ $(OFLAGS) -o $@ $<

