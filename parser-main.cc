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
#include <unistd.h>

#include "ast.hpp"
#include "c.tab.hpp"

extern int yy_flex_debug;                // Control Flex debugging (set to 1 to turn on)
std::istream *g_istream = &std::cin;  // istream being lexed/parsed
const char *g_filename = "<stdin>";   // Path to current file being lexed/parsed

int c_yydebug;           // Control Bison debugging (set to 1 to turn on)
extern zc::TranslationUnit *g_AST_root;
extern int omerrs;              // Number of lexing and parsing errors
extern int yyparse();         // Entry point to the parser

namespace {

void usage(const char *program) {
  std::cerr << "Usage: " << program << " [-lp]" << std::endl;
}

}

int main(int argc, char *argv[]) {
  yy_flex_debug = 0;
  c_yydebug = 0;

  int c;
  opterr = 0;  // getopt shouldn't print any messages
  while ((c = getopt(argc, argv, "lpscrgtTOo:h")) != -1) {
    switch(c) {
#ifdef DEBUG
      case 'l':
        yy_flex_debug = 1;
        break;
      case 'p':
        cool_yydebug = 1;
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


  yyparse();
  if (omerrs != 0) {
    std::cerr << "Compilation halted due to lex and parse errors" << std::endl;
    exit(1);
  }

  g_AST_root->Dump(std::cout, 0);
  // g_AST_root->Dump(std::cout, 0, false /* No types dumped as this stage */);

  return 0;
}

