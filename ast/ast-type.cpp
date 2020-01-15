#include <cassert>
#include <stdexcept>
#include <algorithm>

#include "ast.hpp"
#include "asm.hpp"

extern const char *g_filename;
extern zc::SemantError g_semant_error;

namespace zc {

   int TaggedType::unique_id_counter_ = 0;

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
      for (auto param : *params()) {
         param->DumpNode(os);
      }
      os << ")";
   }

   const FunctionType *PointerType::get_callable() const {
         return dynamic_cast<const FunctionType *>(pointee()); /* beautiful */      
   }

   std::ostream& operator<<(std::ostream& os, ASTType::Kind kind) {
      using Kind = ASTType::Kind;
      os << [](Kind kind) {
               switch (kind) {
               case Kind::TYPE_VOID: return "void";
               case Kind::TYPE_INTEGRAL: return "INTEGRAL";
               case Kind::TYPE_POINTER: return "POINTER";
               case Kind::TYPE_FUNCTION: return "FUNCTION";
               case Kind::TYPE_TAGGED: return "TAGGED";
               case Kind::TYPE_ARRAY: return "ARRAY";
               }
            }(kind);
      return os;
   }

   std::ostream& operator<<(std::ostream& os, TaggedType::TagKind kind) {
      using Kind = TaggedType::TagKind;
      switch (kind) {
      case Kind::TAG_STRUCT: os << "struct"; break;
      case Kind::TAG_UNION: os << "union"; break;
      case Kind::TAG_ENUM: os << "enum"; break;
      }
      return os;
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

   void Enumerator::DumpNode(std::ostream& os) const {
      os << "'" << *id()->id() << "'";
   }

   void Enumerator::DumpChildren(std::ostream& os, int level, bool with_types) const {
      if (val() != nullptr) {
         val()->Dump(os, level, with_types);
      }
   }

   void TaggedType::DumpNode(std::ostream& os) const {
      os << tag_kind() << "'" << *tag() << "'";
   }


   /*** GET DECLARABLES ***/

   void PointerType::get_declarables(Declarations* output) const {
      /* declarable type might appear somewhere in pointee */
      pointee()->get_declarables(output);
   }
   
   void FunctionType::get_declarables(Declarations* output) const {
      /* declarable types might appear in the function parameters or return value */
      return_type()->get_declarables(output);

#if 0
      for (auto param : *params()) {
         param->get_declarables(output);
      }
#endif
   }

   void DeclarableType::get_declarables(Declarations* output) const {
      output->push_back(TypeDeclaration::Create((DeclarableType *) this));
   }

   void CompoundType::get_declarables(Declarations* output) const {
      TaggedType::get_declarables(output);
      if (membs()) {
         for (auto memb : *membs()) {
            auto type_decl = dynamic_cast<TypeDeclaration *>(memb);
            if (type_decl) {
               output->push_back(type_decl);
            }
         }
      }
   }

   void ArrayType::get_declarables(Declarations* output) const {
      elem()->get_declarables(output);
   }

   ASTType *NamedType::Type(SemantEnv& env) const {
      Declaration *decl = env.symtab().Lookup(sym());
      assert(decl);
      auto type_decl = dynamic_cast<TypenameDeclaration *>(decl);
      assert(type_decl);
      return type_decl->type();
   }

   /*** TYPE RESOLUTION ***/

   ASTType *FunctionType::TypeResolve(SemantEnv& env) {
      return_type_ = return_type_->TypeResolve(env);
      for (auto param_decl : *params()) {
         param_decl->TypeResolve(env);
      }
      return this;
   }

   ASTType *CompoundType::TypeResolve(SemantEnv& env) {
       if (membs()) {
           for (auto memb : *membs()) {
               memb->TypeResolve(env);
           }
       }
      return this;
   }

   ASTType *NamedType::TypeResolve(SemantEnv& env) {
      Declaration *decl = env.symtab().Lookup(sym());
      if (decl == nullptr) {
         env.error()(g_filename, this) <<  "undeclared symbol '" << *sym() << "'"
                                       << std::endl;
         return IntegralType::Create(IntegralType::IntKind::SPEC_INT, loc());
      }
      
      auto typename_decl = dynamic_cast<TypenameDeclaration *>(decl);
      if (typename_decl == nullptr) {
         env.error()(g_filename, this) << "symbol '" << *sym() << "' declared with conflicting "
                                       << "type" << std::endl;
         return IntegralType::Create(IntegralType::IntKind::SPEC_INT, loc());
      }

      return typename_decl->type();
   }

 }
