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

   std::ostream& operator<<(std::ostream& os, TypeSpec spec) {
      switch (spec) {
      case TypeSpec::TYPE_VOID:      os << "VOID"; return os;
      case TypeSpec::TYPE_CHAR:      os << "CHAR"; return os;
      case TypeSpec::TYPE_SHORT:     os << "SHORT"; return os;
      case TypeSpec::TYPE_INT:       os << "INT"; return os;
      case TypeSpec::TYPE_LONG:      os << "LONG"; return os;
      case TypeSpec::TYPE_LONG_LONG: os << "LONG_LONG"; return os;
      }
   }
   
   void DeclSpecs::DumpChildren(std::ostream& os, int level, bool with_types) const {
      type_specs()->Dump(os, level, with_types);
   }

   void Decl::DumpType(std::ostream& os) const {
      Type()->DumpNode(os);
   }

   Identifier *Decl::id() const { return declarator_->id(); }

   Symbol *ExternalDecl::sym() const { return decl()->sym(); }
   Symbol *Decl::sym() const { return id()->id(); }

}
