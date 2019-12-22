#include "ast.hpp"

namespace zc {

   void FunctionDef::DumpNode(std::ostream& os) const { os << "FunctionDef"; }
   void FunctionDef::DumpChildren(std::ostream& os, int level) const {
         decl()->Dump(os, level);
         comp_stat()->Dump(os, level);      
   }

   void PointerDeclarator::DumpNode(std::ostream& os) const {
      os << "PointerDeclarator " << depth_;
   }

   void Identifier::DumpNode(std::ostream& os) const {
      os << "Identifier " << "\"" << *id_ << "\"";
   }

   void BasicDeclarator::DumpChildren(std::ostream& os, int level) const { id_->Dump(os, level); }

   void FunctionDeclarator::DumpChildren(std::ostream& os, int level) const  {
      declarator_->Dump(os, level);
      params_->Dump(os, level);
   }

   void Decl::DumpChildren(std::ostream& os, int level) const {
      specs_->Dump(os, level);
      declarator_->Dump(os, level);
   }

   std::ostream& operator<< (std::ostream& os, const TypeSpec& spec) {
      switch (spec) {
      case TYPE_VOID:  os << "VOID"; return os;
      case TYPE_CHAR:  os << "CHAR"; return os;
      case TYPE_SHORT: os << "SHORT"; return os;
      case TYPE_INT:   os << "INT"; return os;
      case TYPE_LONG:  os << "LONG"; return os;
      case TYPE_LL:    os << "LONG LONG"; return os;
      default:         os << "(invalid)"; return os;
      }
   }
   
   void DeclSpecs::DumpChildren(std::ostream& os, int level) const {
      type_specs_->Dump(os, level);
   }

   Identifier *Decl::id() const { return declarator_->id(); }
   
}
