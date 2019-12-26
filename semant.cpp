#include <optional>

#include "ast.hpp"
#include "symtab.hpp"
#include "semant.hpp"

extern const char *g_filename;

namespace zc {

   bool g_semant_debug = false;
   SemantError g_semant_error(std::cerr);

   void Semant(TranslationUnit *root) {
      SemantEnv env(g_semant_error);
      root->TypeCheck(env);
   }

   std::ostream& SemantError::operator()(const char *filename, const ASTNode *node) {
      errors_++;
      os_ << filename << ":" << node->loc() << ": ";
      return os_;
   }

   TypeSpec TypeSpecs::TypeCombine() const {
      return TypeCombine(nullptr);
   }

   TypeSpec TypeSpecs::TypeCombine(SemantEnv *env) const {
      auto specs = specs_;
      
      /* check if any specs given */
      if (specs.size() == 0) {
         if (env) {
            env->error()(g_filename, this) << "declaration missing type specifier" << std::endl;
         }
         return TypeSpec::TYPE_INT;
      }

      /* reduce long long -> long_long */
      if (specs.count(TypeSpec::TYPE_LONG) == 2) {
         auto p = specs_.equal_range(TypeSpec::TYPE_LONG);
         specs.erase(p.first, p.second);
         specs.insert(TypeSpec::TYPE_LONG_LONG);
      }

      /* remove redundant INT */
      std::array<TypeSpec,3> int_compat = {TypeSpec::TYPE_SHORT, TypeSpec::TYPE_LONG,
                                           TypeSpec::TYPE_LONG_LONG};
      if (specs.find(TypeSpec::TYPE_INT) != specs.end() && specs.size() >= 2 &&
          std::any_of(int_compat.begin(), int_compat.end(),
                      [&](TypeSpec spec) {
                         return specs.find(spec) != specs.end();
                      })) {
         specs.erase(specs.find(TypeSpec::TYPE_INT)); /* remove exactly one INT */
      }

      /* NOTE: There should now only be one type left, any of VOID, CHAR, SHORT, INT, LONG, LL. */
      /* check for extraneous types */
      if (specs.size() != 1) {
         if (env) {
            env->error()(g_filename, this) << "too many or incompatible type specifiers given"
                                           << std::endl;
         }
      }

      return *specs.begin();
   }

   /*** TYPE CHECK ***/

   void BasicType::TypeCheck(SemantEnv& env) {
      if (type_spec() == TypeSpec::TYPE_VOID) {
         env.error()(g_filename, this) << "incomplete type 'void'" << std::endl;
      }
   }

   void PointerType::TypeCheck(SemantEnv& env) {
      pointee()->TypeCheck(env);
   }

   void FunctionType::TypeCheck(SemantEnv& env) {
      /* NOTE: function's don't need to have complete return types (i.e. they can be 'void').
       * Therefore, we need not type-check the function's return type.
       */

      /* Functions cannot return functions, only function pointers. Verify this: */
      if (return_type()->kind() == Kind::TYPE_FUNCTION) {
         env.error()(g_filename, this) << "functions cannot return other functions, "
                                       << "only function pointers" << std::endl;
         /* error recovery: transform return type into function */
         return_type_ = PointerType::Create(1, return_type(), return_type()->loc());
      }
      
      params()->TypeCheck(env);
   }

   void Types::TypeCheck(SemantEnv& env) {
      ASTNodeVec::TypeCheck(env);
   }
   
   void TranslationUnit::TypeCheck(SemantEnv& env) {
      env.symtab().EnterScope();
      decls()->TypeCheck(env);
   }

   void DeclSpecs::TypeCheck(SemantEnv& env) {
      type_specs()->TypeCheck(env);
   }

   void TypeSpecs::TypeCheck(SemantEnv& env) {
      /* make sure all type specifiers present are compatible */
      TypeCombine(&env);
   }

   void Decl::TypeCheck(SemantEnv& env, int level) {
      specs()->TypeCheck(env);
      declarator()->TypeCheck(env, level);

      if (level > 0) {
         /* check for previous declarations in scope */
         Symbol *sym = id()->id();
         if (env.symtab().Probe(sym) != nullptr) {
            /* ERROR: symbol already defined in this scope. */
            env.error()(g_filename, this) << "redefinition of '" << sym << "'" << std::endl;
            return;
         }
         
         ASTType *type = Type();
         type->TypeCheck(env);
         
         /* add symbol to scope */
         env.symtab().AddToScope(sym, type);
      }
   }

   void FunctionDeclarator::TypeCheck(SemantEnv& env, int level) {
      declarator()->TypeCheck(env, level - 1);
      params()->TypeCheck(env, (level == 2) ? 1 : 0);
   }

   void FunctionDef::TypeCheck(SemantEnv& env) {
      decl()->TypeCheck(env, 2);          /* not abstract -- enter types in scoped symbol table */
      env.symtab().EnterScope();          /* push scope for fn body */
      comp_stat()->TypeCheck(env, false); /* function body doesn't get new scope */
      env.symtab().ExitScope();           /* pop scope */
   }

   void CompoundStat::TypeCheck(SemantEnv& env, bool scoped) {
      if (scoped) {
         env.symtab().EnterScope();
      }
      
      decls()->TypeCheck(env);
      stats()->TypeCheck(env);

      if (scoped) {
         env.symtab().ExitScope();
      }
   }

   void ExprStat::TypeCheck(SemantEnv& env) {
      expr()->TypeCheck(env);
   }


   void AssignmentExpr::TypeCheck(SemantEnv& env) {
      lhs_->TypeCheck(env);
      rhs_->TypeCheck(env);

      if (lhs_->expr_kind() != ExprKind::EXPR_LVALUE) {
         env.error()(g_filename, this) << "left-hand side of expression is not an lvalue"
                                       << std::endl;
      }

      if (!lhs_->type()->TypeCoerce(rhs_->type())) {
         env.error()(g_filename, this) << "assignment to incompatible type" << std::endl;
      }
      
      type_ = lhs_->type();
   }


   void UnaryExpr::TypeCheck(SemantEnv& env) {
      expr_->TypeCheck(env);

      ASTType *type = expr_->type();
      switch (kind_) {
      case Kind::UOP_ADDR: /* requires LVALUE */
         if (expr_->expr_kind() != ExprKind::EXPR_LVALUE) {
            env.error()(g_filename, this) << "attempt to take address of non-lvalue expression"
                                          << std::endl;
         }
         type_ = type->Address();
         break;
         
      case Kind::UOP_DEREFERENCE:
         if ((type_ = type->Dereference()) == nullptr) {
            env.error()(g_filename, this) << "attempt to dereference non-pointer type" << std::endl;
            type_ = type;
         }
         break;
         
      case Kind::UOP_POSITIVE:
      case Kind::UOP_NEGATIVE:
      case Kind::UOP_BITWISE_NOT:
      case Kind::UOP_LOGICAL_NOT:
         /* requires integral type */
         
         if (!expr_->type()->is_integral()) {
            env.error()(g_filename, this) << "positive/negative sign verboten for non-integral type"
                                          << std::endl;
         }
         type_ = expr_->type();
         break;
      }
   }

   void BinaryExpr::TypeCheck(SemantEnv& env) {
      lhs_->TypeCheck(env);
      rhs_->TypeCheck(env);
      
      switch (kind()) {
      case Kind::BOP_LOGICAL_AND:
      case Kind::BOP_BITWISE_AND:
      case Kind::BOP_LOGICAL_OR:
      case Kind::BOP_BITWISE_OR:
      case Kind::BOP_BITWISE_XOR:
      case Kind::BOP_EQ:
      case Kind::BOP_NEQ:
      case Kind::BOP_LT:
      case Kind::BOP_LEQ:
      case Kind::BOP_GT:
      case Kind::BOP_GEQ:
      case Kind::BOP_PLUS:
      case Kind::BOP_MINUS:
      case Kind::BOP_TIMES:
      case Kind::BOP_DIVIDE:
      case Kind::BOP_MOD:
         {
            bool lhs_int = lhs_->type()->is_integral();
            bool rhs_int = rhs_->type()->is_integral();

            if (lhs_int && rhs_int) {
               type_ = dynamic_cast<const BasicType *>
                  (lhs_->type())->Max(dynamic_cast<const BasicType *>(rhs_->type()));
            } else {
               if (!lhs_int) {
                  env.error()(g_filename, this) << "left-hand expression in binary operation "
                                                << "is of non-integral type" << std::endl;
               }
               if (!rhs_int) {
                  env.error()(g_filename, this) << "right-hand expression in binary operation "
                                                << "is of non-integral type" << std::endl;
               }
               type_ = lhs_->type();
            }
         }
      }
   }

   void LiteralExpr::TypeCheck(SemantEnv& env) {
      type_ = BasicType::Create(TypeSpec::TYPE_LONG_LONG, loc());
   }

   void StringExpr::TypeCheck(SemantEnv& env) {
      type_ = PointerType::Create(1, BasicType::Create(TypeSpec::TYPE_CHAR, loc()), loc());
   }

   void IdentifierExpr::TypeCheck(SemantEnv& env) {
      if ((type_ = env.symtab().Lookup(id()->id())) == nullptr) {
         env.error()(g_filename, this) << "use of undeclared identifier '" << id()->id()
                                       << "'" << std::endl;
         type_ = BasicType::Create(TypeSpec::TYPE_INT, loc());
      }
   }


   
   
    /*** DEREFERENCE ***/
   

    /*** EXPRESSION KIND (LVALUE or RVALUE) ***/
    ASTExpr::ExprKind AssignmentExpr::expr_kind() const {
       /* assignments always produce rvalues */
       return ExprKind::EXPR_RVALUE;
    }

    ASTExpr::ExprKind UnaryExpr::expr_kind() const {
       switch (kind_) {
       case Kind::UOP_ADDR:
          return ExprKind::EXPR_RVALUE;
       
       case Kind::UOP_DEREFERENCE:
          return ExprKind::EXPR_LVALUE;
       
       case Kind::UOP_POSITIVE:
       case Kind::UOP_NEGATIVE:
       case Kind::UOP_BITWISE_NOT:
       case Kind::UOP_LOGICAL_NOT:
          return ExprKind::EXPR_RVALUE;
       }
    }

    ASTExpr::ExprKind BinaryExpr::expr_kind() const {
       switch (kind_) {
       case Kind::BOP_LOGICAL_AND:
       case Kind::BOP_BITWISE_AND:
       case Kind::BOP_LOGICAL_OR:
       case Kind::BOP_BITWISE_OR:
       case Kind::BOP_BITWISE_XOR:
       case Kind::BOP_EQ:
       case Kind::BOP_NEQ:
       case Kind::BOP_LT:
       case Kind::BOP_LEQ:
       case Kind::BOP_GT:
       case Kind::BOP_GEQ:
       case Kind::BOP_PLUS:
       case Kind::BOP_MINUS:
       case Kind::BOP_TIMES:
       case Kind::BOP_DIVIDE:
       case Kind::BOP_MOD:
          return ExprKind::EXPR_RVALUE;
       }
    }

    ASTExpr::ExprKind LiteralExpr::expr_kind() const {
       return ExprKind::EXPR_RVALUE;
    }

    ASTExpr::ExprKind StringExpr::expr_kind() const {
       return ExprKind::EXPR_RVALUE;
    }

    ASTExpr::ExprKind IdentifierExpr::expr_kind() const {
       return ExprKind::EXPR_LVALUE;
    }

    /*** TYPE EQUALITY ***/
    bool BasicType::TypeEq(const ASTType *other) const {
       const BasicType *basic_other = dynamic_cast<const BasicType *>(other);
       if (basic_other == nullptr) {
          return false;
       } else {
          return this->type_spec() == basic_other->type_spec();
       }
    }

    bool PointerType::TypeEq(const ASTType *other) const {
       const PointerType *ptr_other = dynamic_cast<const PointerType *>(other);
       if (ptr_other == nullptr) {
          return false;
       } else {
          return this->depth() == ptr_other->depth() && pointee()->TypeEq(ptr_other->pointee());
       }
    }

    bool FunctionType::TypeEq(const ASTType *other) const {
       const FunctionType *fn_other = dynamic_cast<const FunctionType *>(other);
       if (fn_other == nullptr) {
          return false;
       } else {
          return return_type()->TypeEq(fn_other->return_type()) &&
             params()->TypeEq(fn_other->params());
       }
    }

    bool Types::TypeEq(const Types *others) const {
       if (this->vec_.size() != others->vec_.size()) {
          return false;
       }
    
       for (auto this_it = this->vec_.begin(), others_it = others->vec_.begin();
            this_it != this->vec_.end();
            ++this_it, ++others_it) {
          if (!(*this_it)->TypeEq(*others_it)) {
             return false;
          }
       }
    
       return true;
    }

    /*** TYPE COERCION ***/
    bool BasicType::TypeCoerce(const ASTType *from) const {
       const BasicType *basic_from = dynamic_cast<const BasicType *>(from);
       if (basic_from == nullptr) {
          return false;
       } else {
          return IsIntegral(this->type_spec()) && IsIntegral(basic_from->type_spec());
      }
   }

   bool PointerType::TypeCoerce(const ASTType *from) const {
      /* -1. Check if types are equal. */
      if (this->TypeEq(from)) {
         return true;
      }
      
      /* 0. Verify `from' is a pointer. */
      const PointerType *from_ptr = dynamic_cast<const PointerType *>(from);
      if (from_ptr == nullptr) {
         return false; /* can never implicitly cast from non-pointer type to pointer type */
      }
      
      /* 1. Check if coercing to 'void *'. */
      ASTType *void_ptr = PointerType::Create(1,
                                              BasicType::Create(TypeSpec::TYPE_VOID, loc()),
                                              loc());
      if (this->TypeEq(void_ptr)) {
         /* Verify that pointer is basic (i.e. not a function). */
         if (from_ptr->pointee()->kind() != Kind::TYPE_BASIC) {
            return false; /* cannot cast from function pointer to void pointer */
         } else {
            return true; /* but you can cast any basic pointer to a void pointer. */
         }
      }

      return false; /* cannot coerce since pointers are non-identical and `to' is not void ptr */
   }
   
   bool FunctionType::TypeCoerce(const ASTType *from) const {
      return TypeEq(from);
   }

   /*** JOIN POINTERS ***/

   void PointerDeclarator::JoinPointers() {
      if (declarator()->kind() == Kind::DECLARATOR_POINTER) {
         /* merge consecutive pointers */
         PointerDeclarator *other_ptr = dynamic_cast<PointerDeclarator *>(declarator());
         depth_ += other_ptr->depth_;
         declarator_ = other_ptr->declarator_;
      }

      declarator()->JoinPointers();
   }

   void FunctionDeclarator::JoinPointers() {
      declarator()->JoinPointers();
      params()->JoinPointers();
   }

   void Decls::JoinPointers() {
      std::for_each(vec_.begin(), vec_.end(),
                    [](Decl *decl) {
                       decl->JoinPointers();
                    });
   }

   void Decl::JoinPointers() {
      declarator()->JoinPointers();
   }

   ASTType *BasicDeclarator::Type(ASTType *type) const {
      return type;
   }

   /* This is so fucking confusing. */
   ASTType *PointerDeclarator::Type(ASTType *type) const {
      switch (declarator()->kind()) {
      case Kind::DECLARATOR_BASIC:
         /* basic pointer */
         return PointerType::Create(depth(), declarator()->Type(type), loc());
         
      case Kind::DECLARATOR_POINTER:
         /* this shouldn't happen */
         abort();
         
      case Kind::DECLARATOR_FUNCTION:
         /* tricky: pointer is actualy part of the return value type */
         return declarator()->Type(PointerType::Create(depth(), type, loc()));
      }
   }

   ASTType *FunctionDeclarator::Type(ASTType *type) const {
      /* NOTE: _type_ represents the (possibly partial) return value of this function. */
      Types *param_types = params()->Type();

      /* WHOA -- if this works, it's beautiful. */
      switch (declarator()->kind()) {
      case Kind::DECLARATOR_BASIC:
         return FunctionType::Create(type, param_types, loc());
         
      case Kind::DECLARATOR_POINTER:
         /* Actually a function pointer -- but primarily a pointer. */
         return declarator()->Type(FunctionType::Create(type, param_types, loc()));
         
      case Kind::DECLARATOR_FUNCTION:
         /* This function is actually the RETURN TYPE of another function declared
          * _declarator()_. */
         return declarator()->Type(FunctionType::Create(type, param_types, loc()));
      }
   }

   ASTType *Decl::Type() const {
      ASTType *init_type = BasicType::Create(specs()->type_spec(), loc());
      return declarator()->Type(init_type);
   }

   Types *Decls::Type() const {
      Types::Vec type_vec(vec_.size());
      std::transform(vec_.begin(), vec_.end(), type_vec.begin(),
                     [](Decl *decl) -> ASTType * {
                        return decl->Type();
                     });
      return Types::Create(type_vec, loc());
   }


   /*** ADDRESS OF TYPE ***/
   ASTType *BasicType::Address() {
      return PointerType::Create(1, this, loc());
   }
   
   ASTType *PointerType::Address() {
      return PointerType::Create(depth() + 1, pointee(), loc());
   }

   ASTType *FunctionType::Address() {
      return this;
   }

   /*** DEREFERENCE TYPE ***/
   ASTType *BasicType::Dereference(SemantEnv *env) {
      if (env) {
         env->error()(g_filename, this) << "cannot derereference type" << std::endl;
      }

      return nullptr;
   }

   ASTType *PointerType::Dereference(SemantEnv *env) {
      if (depth() == 1) {
         return pointee();
      } else {
         return PointerType::Create(depth() - 1, pointee(), loc());
      }
   }

   ASTType *FunctionType::Dereference(SemantEnv *env) {
      return this; /* function types are infinitely dereferencable */
   }

   /*** COMBINE ***/
   BasicType *BasicType::Max(const BasicType *with) const {
      return BasicType::Create(::zc::Max(this->type_spec(), with->type_spec()), loc());
   }

}
