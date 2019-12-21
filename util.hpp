#include <iostream>

#ifndef __UTIL_HPP
#define __UTIL_HPP

#include "c.tab.hpp"

namespace zc {

   const char *token_to_str(int tok);
   void dump_token(std::ostream& out, int lineno, int token, YYSTYPE yylval);
   void print_token(std::ostream& out, int token, YYSTYPE yylval);   

   inline std::ostream& indent(std::ostream& out, int padding) {
      if (padding > 80)
         padding = 80;
      else if (padding < 0)
         return out;
      std::fill_n(std::ostream_iterator<char>(out), padding, ' ');
      return out;
   }

}

#endif
