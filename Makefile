CXXFLAGS += -std=c++2a -I . -Wno-register -g -DEZ80
LDFLAGS += -L/usr/local/opt/flex/lib

AST_HDRS = ast.hpp $(wildcard ast/*.hpp)
AST_SRCS = $(wildcard ast/*.cpp)
AST_OBJS = $(AST_SRCS:.cpp=.o)

ASM_HDRS = asm.hpp $(wildcard asm/*.hpp)
ASM_SRCS = $(wildcard asm/*.cpp)
ASM_OBJS = $(ASM_SRCS:.cpp=.o)

BISON_FLAGS = -v -d

BISON_SRC = c.tab.cpp
BISON_OBJ = $(BISON_SRC:.cpp=.o)
BISON_HDR = $(BISON_SRC:.cpp=.hpp)

FLEX_SRC = lex.yy.c
FLEX_OBJ = $(FLEX_SRC:.c=.o)

TOP_SRCS = cgen.cpp semant.cpp env.cpp symtab.cpp util.cpp optim.cpp peephole.cpp
TOP_HDRS = $(wildcard *.hpp)
TOP_OBJS = $(TOP_SRCS:.cpp=.o)

HDRS = $(AST_HDRS) $(BISON_HDR) $(TOP_HDRS)
OBJS = $(AST_OBJS) $(ASM_OBJS) $(TOP_OBJS)

CLEAN_OBJS = $(shell find . -name "*.o")
CLEAN_SRCS = $(BISON_SRC) $(FLEX_SRC)
CLEAN_HDRS = $(BISON_HDR)

.PHONY: all
all: c.tab.o flex lexer-main parser-main semant-main cgen-main $(AST_OBJS) $(ASM_OBJS)

.PHONY: bison
bison: $(BISON_OBJ)

$(BISON_SRC) $(BISON_HDR): c.ypp $(AST_HDRS)
	bison $(BISON_FLAGS) $<
$(BISON_OBJ): $(BISON_SRC)
	c++ -c $(CXXFLAGS) -o $@ $^

.PHONY: flex
flex: $(FLEX_OBJ)

$(FLEX_SRC): c.l $(AST_HDRS)
	flex $<

$(FLEX_OBJ): $(FLEX_SRC)
	c++ -c $(CXXFLAGS) -o $@ $^

asm/%.o: asm/%.cpp $(ASM_HDRS)
	c++ -c $(CXXFLAGS) -o $@ $<

%.o: %.cpp $(HDRS)
	c++ -c $(CXXFLAGS) -o $@ $<

%.o: %.cc $(HDRS)
	c++ -c $(CXXFLAGS) -o $@ $<

%.o: %.c $(HDRS)
	c++ -c $(CXXFLAGS) -o $@ $<

lexer-main: lexer-main.o $(FLEX_OBJ) $(OBJS)
	c++ $(LDFLAGS) -lfl -o $@ $^

parser-main: parser-main.o $(FLEX_OBJ) $(BISON_OBJ) $(OBJS)
	c++ $(LDFLAGS) -lfl -o $@ $^

semant-main: semant-main.o $(FLEX_OBJ) $(BISON_OBJ) $(OBJS)
	c++ $(LDFLAGS) -lfl -o $@ $^

cgen-main: cgen-main.o $(FLEX_OBJ) $(BISON_OBJ) $(OBJS)
	c++ $(LDFLAGS) -lfl -o $@ $^

.PHONY: clean
clean:
	rm -f $(CLEAN_OBJS) $(CLEAN_SRCS) $(CLEAN_HDRS)
