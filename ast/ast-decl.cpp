#include "ast.hpp"
#include "util.hpp"

namespace zc {

   template <> const char *ExternalDecls::name() const { return "ExternalDecls"; }      

   void ExternalDecl::DumpChildren(std::ostream& os, int level, bool with_types) const {
      decl()->Dump(os, level, with_types);
   }
   
   void FunctionDef::DumpNode(std::ostream& os) const { os << "FunctionDef"; }
   void FunctionDef::DumpChildren(std::ostream& os, int level, bool with_types) const {
      ExternalDecl::DumpChildren(os, level, with_types);
      comp_stat()->Dump(os, level, with_types);      
   }

   void PointerDeclarator::DumpNode(std::ostream& os) const {
      os << "PointerDeclarator " << depth_;
   }

   void Identifier::DumpNode(std::ostream& os) const {
      os << "Identifier " << "\"" << *id_ << "\"";
   }

   void BasicDeclarator::DumpChildren(std::ostream& os, int level, bool with_types) const {
      id_->Dump(os, level, with_types);
   }

   void FunctionDeclarator::DumpChildren(std::ostream& os, int level, bool with_types) const  {
      declarator_->Dump(os, level, with_types);
      params_->Dump(os, level, with_types);
   }

   void Decl::DumpChildren(std::ostream& os, int level, bool with_types) const {
      specs_->Dump(os, level, with_types);
      declarator_->Dump(os, level, with_types);
   }

   void IntegralSpec::DumpNode(std::ostream& os) const {
      os << "IntegralSpec " << int_kind();
   }

   void StructSpec::DumpChildren(std::ostream& os, int level, bool with_types) const {
      id()->Dump(os, level, with_types);
      membs()->Dump(os, level, with_types);
   }

   std::ostream& operator<<(std::ostream& os, IntegralSpec::IntKind kind) {
      std::unordered_map<IntegralSpec::IntKind,const char *> map
         {{IntegralSpec::IntKind::SPEC_CHAR, "CHAR"},
          {IntegralSpec::IntKind::SPEC_SHORT, "SHORT"},
          {IntegralSpec::IntKind::SPEC_INT, "INT"},
          {IntegralSpec::IntKind::SPEC_LONG, "LONG"},
          {IntegralSpec::IntKind::SPEC_LONG_LONG, "LONG LONG"},
         };
      os << map[kind];
      return os;
   }

   void Decl::DumpType(std::ostream& os) const {
      Type()->DumpNode(os);
   }

   Identifier *Decl::id() const { return declarator_->id(); }

   Symbol *ExternalDecl::sym() const { return decl()->sym(); }
   Symbol *Decl::sym() const { return id()->id(); }

   bool VoidSpec::Eq(const TypeSpec *other) const {
      return other->kind() == TypeSpec::Kind::SPEC_VOID;
   }

   bool IntegralSpec::Eq(const TypeSpec *other) const {
      return other->kind() == TypeSpec::Kind::SPEC_INTEGRAL &&
         int_kind() == dynamic_cast<const IntegralSpec *>(other)->int_kind();
   }

   bool StructSpec::Eq(const TypeSpec *other) const {
      return other->kind() == TypeSpec::Kind::SPEC_STRUCT &&
         id()->id() == dynamic_cast<const StructSpec *>(other)->id()->id();
   }

   Symbol *StructSpec::sym() const { return id()->id(); }

}
