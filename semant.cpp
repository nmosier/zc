#include "ast.hpp"
#include "symtab.hpp"
#include "semant.hpp"

extern const char *g_filename;

namespace zc {

   bool g_semant_debug = false;
   SemantError g_semant_error(std::cerr);

   void Semant(TranslationUnit *root) {
      SemantEnv env(g_semant_error);
      
      root->TypeCheck(env, true);
   }

   

   std::ostream& SemantError::operator()(const char *filename, const ASTNode *node) {
      errors_++;
      os_ << filename << ":" << node->loc() << ": ";
      return os_;
   }

   bool Decl::TypeCompat(const Decl *other) const {
      return specs()->TypeCompat(other->specs()) && declarator()->TypeCompat(other->declarator());
   }

   bool DeclSpecs::TypeCompat(const DeclSpecs *other) const {
      /* declaration specs are compatible iff type specs are identical 
       * (after preprocessing performed in TypeCheck() */
      return type_specs()->TypeCompat(other->type_specs());
   }

   bool TypeSpecs::TypeCompat(const TypeSpecs *other) const {
      /* ensure preprocessing has been performed */
      assert(this->specs().size() == 1);
      assert(other->specs().size() == 1);

      return this->specs() == other->specs();
   }

   bool PointerDeclarator::TypeCompat(const ASTDeclarator *other) const {
      /* check if _other_ is a pointer declarator; if not, not compatible. */
      const PointerDeclarator *other_ = dynamic_cast<const PointerDeclarator *>(other);
      if (other_ == nullptr) {
         return false;
      }

      if (this->depth_ != other_->depth_) {
         return false;
      }

      return declarator()->TypeCompat(other_->declarator());
   }

   bool BasicDeclarator::TypeCompat(const ASTDeclarator *other) const {
      const BasicDeclarator *other_ = dynamic_cast<const BasicDeclarator *>(other);
      if (other_ == nullptr) {
         return false;
      }

      return true; /* base case */
   }

   bool FunctionDeclarator::TypeCompat(const ASTDeclarator *other) const {
      const FunctionDeclarator *other_ = dynamic_cast<const FunctionDeclarator *>(other);
      if (other_ == nullptr) {
         return false;
      }

      return declarator()->TypeCompat(other_->declarator()) &&
         params()->TypeCompat(other_->params());
   }

   TypeSpec TypeSpecs::TypeCombine(SemantEnv& env) const {
      auto specs = specs_;
      
      /* check if any specs given */
      if (specs.size() == 0) {
         env.error()(g_filename, this) << "declaration missing type specifier" << std::endl;
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
         env.error()(g_filename, this) << "too many or incompatible type specifiers given"
                                          << std::endl;
      }

      return *specs.begin();
   }

   /*** TYPE CHECK ***/

   void TranslationUnit::TypeCheck(SemantEnv& env, bool scoped) {
      env.symtab().EnterScope();
      decls()->TypeCheck(env, true);
   }

   void Decls::TypeCheck(SemantEnv& env, bool scoped) {
      ASTNodeVec::TypeCheck(env, scoped);
   }

   void DeclSpecs::TypeCheck(SemantEnv& env, bool scoped) {
      TypeSpec combined_type_spec = type_specs()->TypeCombine(env);
      type_spec_variant_ = combined_type_spec;
   }

   void TypeSpecs::TypeCheck(SemantEnv& env, bool scoped) {
      env.error()(g_filename, this) << "compiler error: TypeSpecs should have been reduced, "
                                    << "terminating..." << std::endl;
      abort();
   }

   bool Decls::TypeCompat(const Decls *other) const {
      if (this->vec_.size() != other->vec_.size()) {
         return false;
      }

      for (auto this_it = this->vec_.begin(), other_it = other->vec_.begin();
           this_it != this->vec_.end();
           ++this_it, ++other_it) {
         if (!(*this_it)->TypeCompat(*other_it)) {
            return false;
         }
      }

      return true;
   }

   void Decl::TypeCheck(SemantEnv& env, bool scoped) {
      specs()->TypeCheck(env);
      declarator()->TypeCheck(env);
      
      /* check for previous declarations in scope */
      Symbol *sym = id()->id();
      if (env.symtab().Probe(sym) != nullptr) {
         /* ERROR: symbol already defined in this scope. */
         env.error()(g_filename, this) << "redefinition of '" << sym << "'" << std::endl;
         return;
      }

      /* check if type is complete */
      if (specs()->type_spec() == TypeSpec::TYPE_VOID &&
          declarator()->kind() == ASTDeclarator::Kind::DECLARATOR_BASIC) {
         env.error()(g_filename, this) << "declaration has incomplete type 'void'";
      }
      
      /* add symbol to scope */
      env.symtab().AddToScope(id()->id(), this);
      
   }

   void FunctionDeclarator::TypeCheck(SemantEnv& env, bool scoped) {
      declarator()->TypeCheck(env);
      params()->TypeCheck(env);
   }

   void FunctionDef::TypeCheck(SemantEnv& env, bool scoped) {
      decl()->TypeCheck(env);
      env.symtab().EnterScope();      /* push scope for fn body */
      comp_stat()->TypeCheck(env, false); /* function body doesn't get new scope */
      env.symtab().ExitScope();       /* pop scope */
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

   void ExprStat::TypeCheck(SemantEnv& env, bool scoped) {
      expr()->TypeCheck(env);
   }


   void AssignmentExpr::TypeCheck(SemantEnv& env, bool scoped) {
      lhs_->TypeCheck(env);
      rhs_->TypeCheck(env);

      if (lhs_->expr_kind() != ExprKind::EXPR_LVALUE) {
         env.error()(g_filename, this) << "left-hand side of expression is not an lvalue"
                                       << std::endl;
      }
      
      if (!lhs_->type()->TypeCompat(rhs_->type())) {
         env.error()(g_filename, this) << "assignment to incompatible type" << std::endl;
      }
      
      type_ = lhs_->type();
   }


   void UnaryExpr::TypeCheck(SemantEnv& env, bool scoped) {
      expr_->TypeCheck(env);

      Decl *type = expr_->type();
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
         if (expr_->type()->kind() != Decl::Kind::DECL_INTEGRAL) {
            env.error()(g_filename, this) << "positive/negative sign verboten for non-integral type"
                                          << std::endl;
         }
         type_ = type;
         break;
      }
   }

   void BinaryExpr::TypeCheck(SemantEnv& env, bool scoped) {
      lhs_->TypeCheck(env, scoped);
      rhs_->TypeCheck(env, scoped);
      
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
            bool lhs_int = (lhs_->type()->kind() != Decl::Kind::DECL_INTEGRAL);
            bool rhs_int = (rhs_->type()->kind() != Decl::Kind::DECL_INTEGRAL);

            if (lhs_int && rhs_int) {
               TypeSpec lhs_type = lhs_->type()->specs()->type_spec();
               TypeSpec rhs_type = rhs_->type()->specs()->type_spec();
               TypeSpec int_type = Max(lhs_type, rhs_type);
               DeclSpecs *specs = DeclSpecs::Create(int_type, loc_);
               type_ = Decl::Create(DeclSpecs::Create(int_type, loc_),
                                      BasicDeclarator::Create(nullptr, loc_), loc_);
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

   void LiteralExpr::TypeCheck(SemantEnv& env, bool scoped) {
      type_ = Decl::Create(DeclSpecs::Create(TypeSpec::TYPE_LONG_LONG, loc_),
                           BasicDeclarator::Create(nullptr, loc_),
                           loc_);
   }

   void StringExpr::TypeCheck(SemantEnv& env, bool scoped) {
      type_ = Decl::Create(DeclSpecs::Create(TypeSpec::TYPE_CHAR, loc_),
                           PointerDeclarator::Create(1, BasicDeclarator::Create(nullptr, loc_),
                                                     loc_),
                           loc_);
   }

   void IdentifierExpr::TypeCheck(SemantEnv& env, bool scoped) {
      if ((type_ = env.symtab().Lookup(id()->id())) == nullptr) {
         env.error()(g_filename, this) << "use of undeclared identifier '" << id()->id()
                                       << "'" << std::endl;

         /* set default type int */
         type_ = Decl::Create(DeclSpecs::Create(TypeSpec::TYPE_INT, loc_),
                              BasicDeclarator::Create(id_, loc_),
                              loc_);
      }
   }


   
   
   /*** DEREFERENCE ***/
   
   Decl *Decl::Dereference() {
      ASTDeclarator *new_declarator = declarator()->Dereference();
      if (new_declarator == nullptr) {
         return nullptr;
      }

      return Decl::Create(specs_, new_declarator, loc_);
   }
   
   ASTDeclarator *PointerDeclarator::Dereference() {
      if (depth_ == 1) {
         return declarator_; /* no longer a pointer */
      }
      
      return PointerDeclarator::Create(depth_ - 1, declarator_, loc_);
   }

   /*** ADDRESS OF ***/
   Decl *Decl::Address() {
      ASTDeclarator *new_declarator = declarator()->Address();
      if (new_declarator == nullptr) {
         return nullptr;
      }

      return Decl::Create(specs_, new_declarator, loc_);
   }

   ASTDeclarator *PointerDeclarator::Address() {
      return PointerDeclarator::Create(depth_ + 1, declarator_, loc_);
   }

   ASTDeclarator *BasicDeclarator::Address() {
      return PointerDeclarator::Create(1, this, loc_);
   }

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
   
}
