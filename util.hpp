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

   /*** VISITATION & VARIANTS ***/
#if 0
   template <class ...Fs>
   struct visitor : Fs... {
      visitor(Fs const&... fs): Fs{fs}... {}
      using Fs::operator()...;
   };
#endif

   
   template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
   template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
   
}



#endif
