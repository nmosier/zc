#include "ast.hpp"
#include "util.hpp"
#include "cgen.hpp"

namespace zc {


   template <> const char *ExternalDecls::name() const { return "ExternalDecls"; }      

   Symbol *ASTDeclarator::sym() const {
      return id() ? id()->id() : nullptr;
   }
   
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

   void Declaration::DumpChildren(std::ostream& os, int level, bool with_types) const {
      type()->Dump(os, level, with_types);
   }

   void VarDeclaration::DumpNode(std::ostream& os) const {
      os << "VarDeclaration";
   }

   void TypeDeclaration::DumpNode(std::ostream& os) const {
      os << "TypeDeclaration";
   }

   void BasicDeclarator::DumpChildren(std::ostream& os, int level, bool with_types) const {
      id_->Dump(os, level, with_types);
   }

   void FunctionDeclarator::DumpChildren(std::ostream& os, int level, bool with_types) const  {
      declarator_->Dump(os, level, with_types);
      for (auto param : *params_) {
         param->Dump(os, level, with_types);
      }
   }

   void ArrayDeclarator::DumpChildren(std::ostream& os, int level, bool with_types) const {
      declarator()->Dump(os, level, with_types);
      count_expr()->Dump(os, level, with_types);
   }

   void FunctionDeclarator::get_declarables(Declarations* output) const {
#if 0
      for (auto param : *params()) {
         param->get_declarables(output);
      }
#endif
   }

   TypeDeclaration::TypeDeclaration(DeclarableType *type): Declaration(type, type->loc()) {}

   int VarDeclaration::bytes() const { return type()->bytes(); }

   Symbol *ExternalDecl::sym() const {
      const auto var = dynamic_cast<const VarDeclaration *>(decl());
      return var ? var->sym() : nullptr;
   }


}
