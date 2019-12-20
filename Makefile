CXXFLAGS += -std=c++2a -I ast/ -Wno-register -g
LDFLAGS += -L/usr/local/opt/flex/lib

AST_HDRS = ast.hpp $(wildcard ast/*.hpp)

BISON_FLAGS = -d

BISON_SRC = c.tab.cpp
BISON_OBJ = $(BISON_SRC:.cpp=.o)
BISON_HDR = $(BISON_SRC:.cpp=.hpp)

FLEX_SRC = lex.yy.c
FLEX_OBJ = $(FLEX_SRC:.c=.o)

HDRS = $(AST_HDRS) $(BISON_HDR)

.PHONY: all
all: test c.tab.o flex lexer-main

.PHONY: bison
bison: $(BISON_OBJ)

$(BISON_SRC) $(BISON_HDR): c.ypp $(AST_HDRS)
	bison $(BISON_FLAGS) $<

.PHONY: flex
flex: $(FLEX_OBJ)

$(FLEX_SRC): c.l $(HDRS)
	flex $<

$(FLEX_OBJ): $(FLEX_SRC)
	c++ -c $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	c++ -c $(CXXFLAGS) -o $@ $^

%.o: %.cc
	c++ -c $(CXXFLAGS) -o $@ $^

%.o: %.c
	c++ -c $(CXXFLAGS) -o $@ $^

test: test.o $(AST_HDRS)
	c++ $(OFLAGS) -o $@ $<

lexer-main: lexer-main.o lex.yy.o util.o
	c++ $(LDFLAGS) -lfl -o $@ $^
