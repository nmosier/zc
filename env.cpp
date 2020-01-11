#include <string>
#include <iostream>

#include "env.hpp"
#include "ast.hpp"
#include "semant.hpp"
#include "cgen.hpp"

namespace zc {


   template <typename A, typename B, class C>
   void Env<A,B,C>::EnterScope() {
      symtab_.EnterScope();
      tagtab_.EnterScope();
   }

   template <typename A, typename B, class C>   
   void Env<A,B,C>::ExitScope() {
      symtab_.EnterScope();
      tagtab_.EnterScope();
   }   

   template class Env<ASTType, TaggedType, SemantExtEnv>;
   template class Env<SymInfo, TagInfo, CgenExtEnv>;
   
}
