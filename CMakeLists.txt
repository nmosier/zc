cmake_minimum_required(VERSION 3.10)
project(zc VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# required libraries & executables
find_package(BISON)
find_package(FLEX 2.5.35 REQUIRED)

add_compile_options("-pedantic;-DEZ80")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

BISON_TARGET(zc_parser c.ypp ${CMAKE_CURRENT_BINARY_DIR}/zc-parser.cpp)
FLEX_TARGET(zc_lexer c.l ${CMAKE_CURRENT_BINARY_DIR}/zc-lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(zc_lexer zc_parser)

set_source_files_properties(
  ${FLEX_zc_lexer_OUTPUTS}
  PROPERTIES
  COMPILE_FLAGS -Wno-register
)

add_subdirectory(ast)
add_subdirectory(asm)

add_library(zc_objs
  OBJECT
  cgen.cpp
  emit.cpp
  env.cpp
  optim.cpp
  peephole.cpp
  ralloc.cpp
  semant.cpp
  symtab.cpp
  util.cpp
)

add_executable(cgen-main
  cgen-main.cc
  ${FLEX_zc_lexer_OUTPUTS}
  ${BISON_zc_parser_OUTPUTS}
  $<TARGET_OBJECTS:zc_objs>
  $<TARGET_OBJECTS:ast_objs>
  $<TARGET_OBJECTS:asm_objs>
)
