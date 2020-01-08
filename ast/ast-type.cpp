#include <cassert>
#include <stdexcept>

#include "ast.hpp"
#include "asm.hpp"

namespace zc {
   IntegralSpec *IntegralSpec::Max(const IntegralSpec *other) const {
      std::array<IntKind,5> ordering {IntKind::SPEC_CHAR,
                                      IntKind::SPEC_SHORT,
                                      IntKind::SPEC_INT,
                                      IntKind::SPEC_LONG,
                                      IntKind::SPEC_LONG_LONG
      };

      auto this_it = std::find(ordering.begin(), ordering.end(), int_kind());
      auto other_it = std::find(ordering.begin(), ordering.end(), other->int_kind());
      
      return IntegralSpec::Create(*std::max(this_it, other_it), loc());
   }

   Size IntegralSpec::size() const {
      std::unordered_map<IntegralSpec::IntKind,Size> map
         {{IntKind::SPEC_CHAR, Size::SZ_CHAR},
          {IntKind::SPEC_SHORT, Size::SZ_SHORT},
          {IntKind::SPEC_INT, Size::SZ_INT},
          {IntKind::SPEC_LONG, Size::SZ_LONG},
          {IntKind::SPEC_LONG_LONG, Size::SZ_LONG_LONG}
         };

      return map[int_kind()];
   }

   Size BasicType::size() const {
      return dynamic_cast<const IntegralSpec *>(type_spec())->size();
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

   void StructType::DumpNode(std::ostream& os) const {
      os << "struct ";
      if (id() != nullptr) {
         os << id()->id();
      }
      os << " { ";
      membs()->DumpNode(os);
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

   Symbol *ASTType::sym() const { return decl()->id()->id(); }


   bool BasicType::is_integral() const {
      return type_spec()->kind() == TypeSpec::Kind::SPEC_INTEGRAL;
   }

   
 }
