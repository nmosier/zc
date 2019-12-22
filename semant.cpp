#include "ast.hpp"
#include "semant.hpp"

extern const char *g_filename;

namespace zc {


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

   TypeSpec TypeSpecs::Combine(SemantEnv& env) const {
      auto specs = specs_;
      
      /* check if any specs given */
      if (specs.size() == 0) {
         env.error()(g_filename, this) << "declaration missing type specifier" << std::endl;
         return TYPE_INT;
      }

      /* reduce long long -> long_long */
      if (specs.count(TYPE_LONG) == 2) {
         auto p = specs_.equal_range(TYPE_LONG);
         specs.erase(p.first, p.second);
         specs.insert(TYPE_LL);
      }

      /* remove redundant INT */
      std::array<TypeSpec,3> int_compat = {TYPE_SHORT, TYPE_LONG, TYPE_LL};
      if (specs.find(TYPE_INT) != specs.end() && specs.size() >= 2 &&
          std::any_of(int_compat.begin(), int_compat.end(),
                      [&](TypeSpec spec) {
                         return specs.find(spec) != specs.end();
                      })) {
         specs.erase(specs.find(TYPE_INT)); /* remove exactly one INT */
      }

      /* NOTE: There should now only be one type left, any of VOID, CHAR, SHORT, INT, LONG, LL. */
      /* check for extraneous types */
      if (specs.size() != 1) {
         env.error()(g_filename, this) << "too many or incompatible type specifiers given"
                                          << std::endl;
      }

      return *specs.begin();
   }

   void TypeSpecs::TypeCheck(SemantEnv& env, bool scoped) {
      TypeSpecs::Combine(env);
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
      } else {
         /* add symbol to scope */
         env.symtab().AddToScope(id()->id(), this);
      }
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

      if (lhs_->kind() != EXPR_LVALUE) {
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
      case UOP_ADDR: /* requires LVALUE */
         if (expr_->kind() != EXPR_LVALUE) {
            env.error()(g_filename, this) << "attempt to take address of non-lvalue expression"
                                          << std::endl;
         }
         type_ = expr_->type()->Address();
         break;
         
      case UOP_DEREFERENCE:
         if ((type_ = expr_->type()->Dereference()) == nullptr) {
            env.error()(g_filename, this) << "attempt to dereference non-pointer type" << std::endl;
            type_ = expr_->type();
         }
         break;
         
      case UOP_POSITIVE:
      case UOP_NEGATIVE:
         /* requires integral type */
         
         
      case UOP_BITWISE_NOT:
      case UOP_LOGICAL_NOT:
      default:
         break;
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

   void AssignmentExpr::ExprKind(SemantEnv& env) {
      /* TODO */
   }

   
}
