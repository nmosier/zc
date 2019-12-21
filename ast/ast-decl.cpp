#include "ast.hpp"

namespace zc {

   void FunctionDef::DumpNode(std::ostream& os) const { os << "FunctionDef"; }
   void FunctionDef::DumpChildren(std::ostream& os, int level) const {
         specs()->Dump(os, level);
         declarator()->Dump(os, level);
         comp_stat()->Dump(os, level);      
   }

   void TypeSpec::DumpNode(std::ostream& os) const {
      const char *type_to_str[] = {"VOID", "CHAR", "SHORT", "INT", "LONG"};

      os << "TypeSpec ";
      if (kind_ < 0 || kind_ >= Kind::NTYPES) {
         os << "(invalid)";
      } else {
         os << type_to_str[kind_];
      }
   }

   void PointerDeclarator::DumpNode(std::ostream& os) const {
      os << "PointerDeclarator " << depth_;
   }

   void Identifier::DumpNode(std::ostream& os) const {
      os << "Identifier " << "\"" << id_ << "\"";
   }

   void BasicDeclarator::DumpChildren(std::ostream& os, int level) const { id_->Dump(os, level); }

   void FunctionDeclarator::DumpChildren(std::ostream& os, int level) const  {
      declarator_->Dump(os, level);
      params_->Dump(os, level);
   }

   
}
