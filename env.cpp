#include <string>
#include <iostream>

#include "env.hpp"
#include "ast.hpp"
#include "semant.hpp"
#include "cgen.hpp"

namespace zc {


   template <typename A, typename B, typename D, class C>
   void Env<A,B,D,C>::EnterScope() {
      symtab_.EnterScope();
      tagtab_.EnterScope();
   }

   template <typename A, typename B, typename D, class C>   
   void Env<A,B,D,C>::ExitScope() {
      symtab_.EnterScope();
      tagtab_.EnterScope();
   }   

   template class Env<VarDeclaration, TaggedType, ASTStat, SemantExtEnv>;
   template class Env<SymInfo, TaggedType, StatInfo, CgenExtEnv>;
   
}
