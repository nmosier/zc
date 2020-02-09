#ifndef __ALG_DAG
#define __ALG_DAG

#include <forward_list>

#include "ast-fwd.hpp"

namespace zc::alg {

   typedef std::forward_list<ASTExpr *> DAGSet;
   
}

#endif
