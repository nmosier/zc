#include "ast.hpp"

namespace zc {

   void UnaryExpr::DumpNode(std::ostream& os) const {
      const char *strs[] =
         {"UOP_ADDR", "UOP_DEREFERENCE", "UOP_POSITIVE", "UOP_NEGATIVE", "UOP_BITWISE_NOT",
          "UOP_LOGICAL_NOT"};
      
      os << "UnaryExpr ";

      if (kind_ < 0 || kind_ >= Kind::NUOPS) {
         os << "(invalid)";
      } else {
         os << strs[kind_];
      }
   }

   const char *BinaryExpr::kindstr() const {
      switch (kind_) {
      case Kind::BOP_LOGICAL_AND: return "BOP_LOGICAL_AND";
      case Kind::BOP_BITWISE_AND: return "BOP_BITWISE_AND";
      case Kind::BOP_LOGICAL_OR:  return "BOP_LOGICAL_OR";
      case Kind::BOP_BITWISE_OR:  return "BOP_BITWISE_OR";
      case Kind::BOP_BITWISE_XOR: return "BOP_BITWISE_XOR";
      case Kind::BOP_EQ:          return "BOP_EQ";
      case Kind::BOP_NEQ:         return "BOP_NEQ";
      case Kind::BOP_LT:          return "BOP_LT";
      case Kind::BOP_LEQ:         return "BOP_LEQ";
      case Kind::BOP_GT:          return "BOP_GT";
      case Kind::BOP_GEQ:         return "BOP_GEQ";
      case Kind::BOP_PLUS:        return "BOP_PLUS";
      case Kind::BOP_MINUS:       return "BOP_MINUS";
      case Kind::BOP_TIMES:       return "BOP_TIMES";
      case Kind::BOP_DIVIDE:      return "BOP_DIVIDE";
      case Kind::BOP_MOD:         return "BOP_MOD";
      default:              return "(invalid)";
      }
   }

   void BinaryExpr::DumpNode(std::ostream& os) const {
      os << "BinaryExpr " << kindstr();
   }

   void LiteralExpr::DumpNode(std::ostream& os) const {
      os << "LiteralExpr " << val_;
   }

   void StringExpr::DumpNode(std::ostream& os) const {
      os << "StringExpr " << str_;
   }

   void IdentifierExpr::DumpChildren(std::ostream& os, int level) const {
      id_->Dump(os, level);
   }

}
