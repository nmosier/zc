#include <iterator>
#include <map>
#include <set>

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

   void BasicTypeSpec::AddTo(DeclSpecs *decl_specs) {
      decl_specs->basic_type_specs.push_back(this);
   }

   void ComplexTypeSpec::AddTo(DeclSpecs *decl_specs) {
      decl_specs->complex_type_specs.push_back(this);
   }

   ASTType *DeclSpecs::Type(SemantError& err) {
      /* check if any typenames are present */
      if (typename_specs.size() > 0) {
         if (typename_specs.size() > 1 || !complex_type_specs.empty() ||
             !basic_type_specs.empty()) {
            err(g_filename, this) << "invalid combination of type specifiers" << std::endl;
         }
         
         /* lookup typename */
         /* TODO */
      }
      
      /* check if any complex types are present */
      if (complex_type_specs.size() > 0) {
         if (complex_type_specs.size() > 1 || basic_type_specs.size() > 0) {
            err(g_filename, this) << "incompatible type specifiers" << std::endl;
         }
         return complex_type_specs.front()->type();
      }

      /* otherwise, all basic. */
      using Kind = BasicTypeSpec::Kind;
      std::multiset<Kind> specs;
      std::transform(basic_type_specs.begin(), basic_type_specs.end(), std::inserter(specs, specs.begin()),
                     [](auto spec) { return spec->kind(); });
      using IntKind = IntegralType::IntKind;
      std::map<std::multiset<Kind>,ASTType *> valid_combos
         {{{Kind::TS_VOID}, VoidType::Create(loc())},
          {{Kind::TS_CHAR}, IntegralType::Create(IntKind::SPEC_CHAR, loc())},
          {{Kind::TS_SHORT}, IntegralType::Create(IntKind::SPEC_SHORT, loc())},
          {{Kind::TS_INT}, IntegralType::Create(IntKind::SPEC_INT, loc())},
          {{Kind::TS_LONG}, IntegralType::Create(IntKind::SPEC_LONG, loc())},
          {{Kind::TS_SHORT, Kind::TS_INT}, IntegralType::Create(IntKind::SPEC_SHORT, loc())},
          {{Kind::TS_LONG, Kind::TS_INT}, IntegralType::Create(IntKind::SPEC_LONG, loc())},
          {{Kind::TS_LONG, Kind::TS_LONG}, IntegralType::Create(IntKind::SPEC_LONG_LONG, loc())},
          {{Kind::TS_LONG, Kind::TS_LONG, Kind::TS_INT}, IntegralType::Create(IntKind::SPEC_LONG_LONG,loc())}
         };

      auto it = valid_combos.find(specs);
      if (it == valid_combos.end()) {
         err(g_filename, this) << "incompatible type specifiers" << std::endl;
         return IntegralType::Create(IntKind::SPEC_INT, loc());
      } else {
         return it->second;
      }
   }

   void StorageClassSpec::AddTo(DeclSpecs *decl_specs) {
      decl_specs->storage_class_specs.push_back(this);
   }

   Symbol *TypenameSpec::sym() const { return id()->id(); }

   void TypenameSpec::AddTo(DeclSpecs *decl_specs) {
      decl_specs->typename_specs.push_back(this);
   }

   void Declaration::TypeResolve(SemantEnv& env) {
      type_ = type_->TypeResolve(env);
   }

}
