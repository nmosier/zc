#include "ast.hpp"
#include "util.hpp"

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

   void BasicDeclarator::DumpChildren(std::ostream& os, int level, bool with_types) const {
      id_->Dump(os, level, with_types);
   }

   void FunctionDeclarator::DumpChildren(std::ostream& os, int level, bool with_types) const  {
      declarator_->Dump(os, level, with_types);
      for (auto param : *params_) {
         param->Dump(os, level, with_types);
      }
   }

   Symbol *ExternalDecl::sym() const { return decl()->sym(); }


   void ArrayDeclarator::DumpChildren(std::ostream& os, int level, bool with_types) const {
      declarator()->Dump(os, level, with_types);
      count_expr()->Dump(os, level, with_types);
   }

   void Declaration::DumpNode(std::ostream& os) const {
      os << "Declaration " << *sym();
      if (is_const()) {
         os << " (const)";
      }
   }

   void Declaration::DumpChildren(std::ostream& os, int level, bool with_types) const {
      type()->Dump(os, level, with_types);
   }

}
