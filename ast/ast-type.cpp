#include <cassert>

#include "ast.hpp"

namespace zc {

   bool IsIntegral(TypeSpec type) {
      switch (type) {
      case TypeSpec::TYPE_VOID:      return false;
      case TypeSpec::TYPE_CHAR:      return true;
      case TypeSpec::TYPE_SHORT:     return true;
      case TypeSpec::TYPE_INT:       return true;
      case TypeSpec::TYPE_LONG:      return true;
      case TypeSpec::TYPE_LONG_LONG: return true;
      case TypeSpec::TYPE_POINTER:   return false;
      case TypeSpec::TYPE_FUNCTION:  return false;
      }
   }

   TypeSpec Max(TypeSpec lhs, TypeSpec rhs) {
      assert(IsIntegral(lhs) && IsIntegral(rhs));
      if (lhs == TypeSpec::TYPE_CHAR || rhs == TypeSpec::TYPE_CHAR) {
         return (lhs == TypeSpec::TYPE_CHAR) ? rhs : lhs;
      }
      if (lhs == TypeSpec::TYPE_SHORT || rhs == TypeSpec::TYPE_SHORT) {
         return (lhs == TypeSpec::TYPE_SHORT) ? rhs : lhs;
      }
      if (lhs == TypeSpec::TYPE_INT || rhs == TypeSpec::TYPE_INT) {
         return (lhs == TypeSpec::TYPE_INT) ? rhs : lhs;
      }
      return lhs; // LONG_LONG
   }

   void BasicType::DumpNode(std::ostream& os) const {
      os << type_spec();
   }

   void PointerType::DumpNode(std::ostream& os) const {
      os << "* ";
      pointee()->DumpNode(os);      
   }

   void FunctionType::DumpNode(std::ostream& os) const {
      os << "(";
      return_type()->DumpNode(os);
      os << ")";
      os << "(";
      params()->DumpNode(os);
      os << ")";
   }

   void Types::DumpNode(std::ostream& os) const {
      std::for_each(vec_.begin(), vec_.end(),
                    [&](const ASTType *type) {
                       type->DumpNode(os);
                       os << ", ";
                    });
   }
}
