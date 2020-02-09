/* Transform expressions (and statements) into DAG. */

#include "alg/alg-dag.hpp"
#include "ast/ast-expr.hpp"

namespace zc::alg {

   ASTExpr *ASTExpr::DAG() {
      alg::DAGSet dagset;
      return transform(&ASTExpr::DAG_aux, dagset);
   }
   
   ASTExpr *ASTExpr::DAG_aux(alg::DAGSet& dagset) {
      auto it = std::find(dagset.begin(), dagset.end(),
                          [&](ASTExpr *dagexpr) {
                             return dagexpr->ExprEq(this);
                          });
      if (it != dagset.end()) {
         /* return expression in DAG set */
         return *it;
      } else {
         dagset.push_back(this);
         return this;
      }
   }
   
}
