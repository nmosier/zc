#include "ast.hpp"
#include "util.hpp"

namespace zc {

   Decl::Kind Decl::kind() const {
      TypeSpec spec_kind = specs()->type_spec();
      ASTDeclarator::Kind declarator_kind = declarator()->kind();

      switch (declarator_kind) {
      case ASTDeclarator::Kind::DECLARATOR_BASIC:
         if (IsIntegral(spec_kind)) {
            return Kind::DECL_INTEGRAL;
         } else {
            return Kind::DECL_VOID;
         }
         
      case ASTDeclarator::Kind::DECLARATOR_POINTER:
         return Kind::DECL_POINTER;
         
      case ASTDeclarator::Kind::DECLARATOR_FUNCTION:
         return Kind::DECL_FUNCTION;
      }
   }
   
   void FunctionDef::DumpNode(std::ostream& os) const { os << "FunctionDef"; }
   void FunctionDef::DumpChildren(std::ostream& os, int level, bool with_types) const {
      decl()->Dump(os, level, with_types);
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
      case TypeSpec::TYPE_POINTER:   os << "POINTER"; return os;
      case TypeSpec::TYPE_FUNCTION:  os << "FUNCTION"; return os;
      }
   }
   
   void DeclSpecs::DumpChildren(std::ostream& os, int level, bool with_types) const {
      std::visit(visitor([&](TypeSpec spec) { indent(os, level); os << spec << std::endl; },
                         [&](TypeSpecs *specs) { specs->Dump(os, level, with_types); }
                         ),
                 type_spec_variant_);
   }

   Identifier *Decl::id() const { return declarator_->id(); }

   void Decl::DumpType(std::ostream& os) const {
      specs()->DumpType(os);
      os << " ";
      declarator()->DumpType(os);
   }

   void DeclSpecs::DumpType(std::ostream& os) const {
      os << type_spec();
   }

   void PointerDeclarator::DumpType(std::ostream& os) const {
      for (int i = 0; i < depth(); ++i) {
         os << '*';
      }

      os << ' ';
      declarator_->DumpType(os);
   }

   void BasicDeclarator::DumpType(std::ostream& os) const {
      /* base case -- omit identifier, so nothing to print */
   }

   void FunctionDeclarator::DumpType(std::ostream& os) const {
      declarator()->DumpType(os);
      os << " (*)";
      params()->DumpType(os);
   }

   void Decls::DumpType(std::ostream& os) const {
      os << '(';
      for (auto i = vec_.size(); i > 0; --i) {
         vec_[i]->DumpType(os);
         if (i > 1) {
            os << ", ";
         }
      }
      os << ')';
         
   }
   
}
