#include "ast.hpp"

namespace zc {

   const char *UnaryExpr::kindstr() const {
      switch (kind_) {
      case Kind::UOP_ADDR: return "UOP_ADDR";
      case Kind::UOP_DEREFERENCE: return "UOP_DEREFERENCE";
      case Kind::UOP_POSITIVE:   return "UOP_POSITIVE";
      case Kind::UOP_NEGATIVE:   return "UOP_NEGATIVE";
      case Kind::UOP_BITWISE_NOT:          return "UOP_BITWISE_NOT";
      case Kind::UOP_LOGICAL_NOT: return "UOP_LOGICAL_NOT";
      }
   }
   
   void UnaryExpr::DumpNode(std::ostream& os) const {
      os << "UnaryExpr " << kindstr();
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
      os << "StringExpr " << *str_;
   }

   void IdentifierExpr::DumpChildren(std::ostream& os, int level, bool with_types) const {
      id_->Dump(os, level, with_types);
   }

   void CallExpr::DumpChildren(std::ostream& os, int level, bool with_types) const {
      fn()->Dump(os, level, with_types);
      params()->Dump(os, level, with_types);
   }

   void CastExpr::DumpChildren(std::ostream& os, int level, bool with_types) const {
      decl()->Dump(os, level, with_types);
      expr()->Dump(os, level, with_types);
   }


   bool BinaryExpr::is_logical() const {
      switch (kind()) {
      case Kind::BOP_LOGICAL_AND:
      case Kind::BOP_LOGICAL_OR:
      case Kind::BOP_EQ:
      case Kind::BOP_NEQ:
      case Kind::BOP_LT:
      case Kind::BOP_LEQ:
      case Kind::BOP_GT:
      case Kind::BOP_GEQ:
         return true;
      default:
         return false;
      }
   }
   
}
