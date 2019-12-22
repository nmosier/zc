
#include "ast.hpp"
#include "util.hpp"

namespace zc {

const int indent_step = 2;

   void ASTNode::Dump(std::ostream& os, size_t level) const {
      indent(os, level);
      os << "#" << loc_ << " ";
      DumpNode(os);
      os << std::endl;
      DumpChildren(os, level + indent_step);
   }


   void ASTBinaryExpr::DumpChildren(std::ostream& os, int level) const {
      lhs_->Dump(os, level);
      rhs_->Dump(os, level);
   }

   void ASTUnaryExpr::DumpChildren(std::ostream& os, int level) const {
      expr_->Dump(os, level);
   }

}
