/*
Copyright (c) 1995,1996 The Regents of the University of California.
All rights reserved.

Permission to use, copy, modify, and distribute this software for any
purpose, without fee, and without written agreement is hereby granted,
provided that the above copyright notice and the following two
paragraphs appear in all copies of this software.

IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

Copyright 2017 Michael Linderman.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 */

#include <iostream>
#include <fstream>
#include <unistd.h>

#include "ast.hpp"
#include "c.tab.hpp"      // Bison-generated file that defines the tokens
#include "util.hpp"

extern int yy_flex_debug;           // Control Flex debugging (set to 1 to turn on)
std::istream* g_istream;            // istream being lexed/parsed
const char *g_filename;          // Path to current file being lexed/parsed

// The lexer keeps this global variable up to date with the line number
// of the current line read from the input.
zc::SourceLoc g_lineno = 1;

// Entry point to the lexer. It returns the next token each time it is called.
extern int yylex();
YYSTYPE yylval; // Not compiled with parser, so must define this.

namespace {

void usage(const char *program) {
  std::cerr << "Usage: " << program << " [-lv] file [...]" << std::endl;
}

}

int main(int argc, char* argv[]) {
  yy_flex_debug = 0;

  int c;
  opterr = 0;  // getopt shouldn't print any messages
  while ((c = getopt(argc, argv, "lpscrgtTOo:h")) != -1) {
    switch(c) {
#ifdef DEBUG
      case 'l':
        yy_flex_debug = 1;
        break;
#endif
      case 'h':
        usage(argv[0]);
        return 0;
      case '?':
        usage(argv[0]);
        return 85;
      default:
        break;
    }
  }

  while (optind < argc) {
    std::ifstream input_stream(argv[optind]);
    if (input_stream.fail()) {
      std::cerr << "Could not open input file: " << argv[optind] << std::endl;
      exit(1);
    }
    g_istream = &input_stream;

    // Reset the line number for the current file
    g_lineno = 1;

    // Scan and print all tokens.
    std::cout << "#name \"" << argv[optind] << "\"" << std::endl;
    int token;
    while ((token = yylex()) != 0) {
     zc::dump_token(std::cout, g_lineno, token, yylval);
    }

    optind++;
  }

  return 0;
}



