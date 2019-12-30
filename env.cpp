#include <string>
#include <iostream>

#include "env.hpp"
#include "ast.hpp"
#include "cgen.hpp"

namespace zc {

   template class Env<ASTType>;
   template class Env<SymInfo>;
   
}
