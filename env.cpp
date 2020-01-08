#include <string>
#include <iostream>

#include "env.hpp"
#include "ast.hpp"
#include "cgen.hpp"

namespace zc {


   template <typename A, typename B, class C>
   void Env<A,B,C>::EnterScope() {
      symtab_.EnterScope();
      structs_.EnterScope();
   }

   template <typename A, typename B, class C>   
   void Env<A,B,C>::ExitScope() {
      symtab_.EnterScope();
      structs_.EnterScope();
   }

   template class Env<ASTType, ASTType, SymbolEnv>;
   template class Env<SymInfo, StructInfo, CgenExtEnv>;
   
}
