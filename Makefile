CXXFLAGS += -std=c++2a -I . -Wno-register -g
LDFLAGS += -L/usr/local/opt/flex/lib

AST_HDRS = ast.hpp $(wildcard ast/*.hpp)
AST_SRCS = $(wildcard ast/*.cpp)
AST_OBJS = $(AST_SRCS:.cpp=.o)

BISON_FLAGS = -d

BISON_SRC = c.tab.cpp
BISON_OBJ = $(BISON_SRC:.cpp=.o)
BISON_HDR = $(BISON_SRC:.cpp=.hpp)

FLEX_SRC = lex.yy.c
FLEX_OBJ = $(FLEX_SRC:.c=.o)

HDRS = $(AST_HDRS) $(BISON_HDR)

CLEAN_OBJS = $(shell find . -name "*.o")
CLEAN_SRCS = $(BISON_SRC) $(FLEX_SRC)
CLEAN_HDRS = $(BISON_HDR)

.PHONY: all
all: test c.tab.o flex lexer-main $(AST_OBJS)

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

lexer-main: lexer-main.o lex.yy.o util.o $(AST_OBJS)
	c++ $(LDFLAGS) -lfl -o $@ $^

.PHONY: clean
clean:
	rm -f $(CLEAN_OBJS) $(CLEAN_SRCS) $(CLEAN_HDRS)
