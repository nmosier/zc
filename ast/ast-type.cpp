#include <cassert>
#include <stdexcept>
#include <algorithm>

#include "ast.hpp"
#include "asm.hpp"

extern const char *g_filename;

namespace zc {

   std::ostream& operator<<(std::ostream& os, IntegralType::IntKind kind) {
      using IntKind = IntegralType::IntKind;
      std::unordered_map<IntegralType::IntKind,const char *> map
         {{IntKind::SPEC_CHAR, "CHAR"},
          {IntKind::SPEC_SHORT, "SHORT"},
          {IntKind::SPEC_INT, "INT"},
          {IntKind::SPEC_LONG, "LONG"},
          {IntKind::SPEC_LONG_LONG, "LONG LONG"},
         };
      os << map[kind];
      return os;
   }   
   
   IntegralType *IntegralType::Max(const IntegralType *other) const {
      std::array<IntKind,5> ordering {IntKind::SPEC_CHAR,
                                      IntKind::SPEC_SHORT,
                                      IntKind::SPEC_INT,
                                      IntKind::SPEC_LONG,
                                      IntKind::SPEC_LONG_LONG
      };

      auto this_it = std::find(ordering.begin(), ordering.end(), int_kind());
      auto other_it = std::find(ordering.begin(), ordering.end(), other->int_kind());
      
      return IntegralType::Create(*std::max(this_it, other_it), loc());
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

   void StructType::DumpNode(std::ostream& os) const {
      os << "struct ";
      if (struct_id() != nullptr) {
         os << struct_id();
      }
      os << " { ";
      if (membs() != nullptr) {
         membs()->DumpNode(os);
      }
      os << " }";
   }

   void Types::DumpNode(std::ostream& os) const {
      std::for_each(vec_.begin(), vec_.end(),
                    [&](const ASTType *type) {
                       type->DumpNode(os);
                       os << ", ";
                    });
   }

   const FunctionType *PointerType::get_callable() const {
         return dynamic_cast<const FunctionType *>(pointee()); /* beautiful */      
   }

   std::ostream& operator<<(std::ostream& os, ASTType::Kind kind) {
      using Kind = ASTType::Kind;
      std::unordered_map<Kind,const char *> map
         {{Kind::TYPE_VOID, "VOID"},
          {Kind::TYPE_INTEGRAL, "INTEGRAL"},
          {Kind::TYPE_POINTER, "POINTER"},
          {Kind::TYPE_FUNCTION, "FUNCTION"},
          {Kind::TYPE_STRUCT, "STRUCT"}
         };

      return os << map[kind];
   }
   
   void IntegralType::DumpNode(std::ostream& os) const {
      os << "IntegralType " << kind();
   }

   void ArrayType::DumpNode(std::ostream& os) const {
      elem()->DumpNode(os);
      os << "[]";
   }

   void VoidType::TypeCheck(SemantEnv& env, bool allow_void) {
      if (!allow_void) {
         env.error()(g_filename, this) << "incomplete 'void' type" << std::endl;
      }
   }

   bool VoidType::TypeEq(const ASTType *other) const {
      return dynamic_cast<const VoidType *>(other) != nullptr;
   }

   bool IntegralType::TypeEq(const ASTType *other) const {
      auto int_other = dynamic_cast<const IntegralType *>(other);
      return int_other == nullptr ? false : int_other->int_kind() == int_kind();
   }

   /* NOTE: This doesn't check if the arrays are of the same size.
    * This may or may not be necessary in the future. */
   bool ArrayType::TypeEq(const ASTType *other) const {
      auto array_other = dynamic_cast<const ArrayType *>(other);
      if (array_other == nullptr) {
         return false;
      } else {
         return elem()->TypeEq(array_other->elem());
      }
   }

   bool ArrayType::TypeCoerce(const ASTType *from) const {
      /* Arrays can decay into pointers, but not vice versa. */
      return TypeEq(from);
   }

   ASTType *IntegralType::Address() {
      return PointerType::Create(1, this, loc());
   }

   void Types::Enscope(SemantEnv& env) {
      for (ASTType *type : vec()) {
         type->Enscope(env);
      }
   }

   ASTType *Types::Lookup(const Symbol *sym) const {
      auto it = std::find_if(vec().begin(), vec().end(),
                             [&](const ASTType *type) -> bool {
                                return type->sym() == sym;
                             });
      return it == vec().end() ? nullptr : *it;
   }


   
 }
