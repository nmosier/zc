
#ifndef __UTIL_HPP
#define __UTIL_HPP


#include <iostream>
#include <vector>
#include <unordered_map>

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
   template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
   template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;


   /*** VEC TO TUPLE ***/
   template <class InputIt, typename... Ts>
   struct vec_to_tupler {
      template <int I>
      static void do_it(std::optional<std::tuple<Ts...>>& t, InputIt begin, InputIt end) {
         if (begin == end) {
            t = std::nullopt;
         } else {
            auto *ptr = dynamic_cast
               <typename std::remove_reference<decltype(std::get<I-1>(*t))>::type>(*begin);
            if (ptr) {
               std::get<I-1>(*t) = ptr;
               do_it<I-1>(t, ++begin, end);
            } else {
               t = std::nullopt;
            }
         }
      }

      template <>
      static void do_it<0>(std::optional<std::tuple<Ts...>>& t, InputIt begin, InputIt end) {
         if (begin != end) {
            t = std::nullopt;
         }
      }
   };

   template <class InputIt, typename... Ts>
   std::optional<std::tuple<Ts...>> vec_to_tuple(InputIt begin, InputIt end) {
      std::optional<std::tuple<Ts...>> t;
      vec_to_tupler<InputIt, Ts...>::template do_it<std::tuple_size<std::tuple<Ts...>>::value>(t, begin, end);
      return t;
   }
}



#endif
