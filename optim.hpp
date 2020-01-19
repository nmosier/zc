#ifndef __OPTIM_HPP
#define __OPTIM_HPP

#include "ast.hpp"

namespace zc {

   void OptimizeAST(TranslationUnit *root);
   void OptimizeIR(CgenEnv& env);
}

#endif
