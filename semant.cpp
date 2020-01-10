#include <unordered_set>

 #include "ast.hpp"
 #include "asm.hpp"
 #include "symtab.hpp"
 #include "semant.hpp"
 #include "c.tab.hpp"
 #include "util.hpp"

 extern const char *g_filename;

 namespace zc {

    bool g_semant_debug = false;
    SemantError g_semant_error(std::cerr);
    SemantEnv g_semant_env(g_semant_error);

    void Semant(TranslationUnit *root) {
       //       SemantEnv env(g_semant_error);
       root->Enscope(g_semant_env);
       root->TypeCheck(g_semant_env);
       root->Descope(g_semant_env);
    }

    std::ostream& SemantError::operator()(const char *filename, const ASTNode *node) {
       return SemantError::operator()(filename, node->loc());
    }
    
    std::ostream& SemantError::operator()(const char *filename, const SourceLoc& loc) {
       errors_++;
       os_ << filename << ":" << loc << ": ";
       return os_;
    }

    static IntegralType::IntKind token_to_intspec(int token) {
       using Kind = IntegralType::IntKind;
       std::unordered_map<int,Kind> map
          {{CHAR, Kind::SPEC_CHAR},
           {SHORT, Kind::SPEC_SHORT},
           {INT, Kind::SPEC_INT},
           {LONG, Kind::SPEC_LONG}
          };
       return map[token];
    }

    /**
     * combine basic type spec tags into one during parsing
     */
    IntegralType *combine_integral_type_specs(std::unordered_multiset<int>& int_specs,
                                              SemantError& error, const SourceLoc& loc) {
       using Kind = IntegralType::IntKind;
       
       switch (int_specs.size()) {
       case 0:
          error(g_filename, loc) << "declaration missing type specifier" << std::endl;
          return IntegralType::Create(Kind::SPEC_INT, loc);
          
       case 1:
          return IntegralType::Create(token_to_intspec(*int_specs.begin()), loc);

       case 2:
          if (int_specs.count(LONG) == 2) {
             return IntegralType::Create(Kind::SPEC_LONG_LONG, loc);
          } else if (int_specs.count(CHAR) > 0) {
             error(g_filename, loc) << "cannot combine 'char' type specifier with other "
                                     << "type specifiers" << std::endl;
             return IntegralType::Create(token_to_intspec(*int_specs.begin()), loc);
          } else if (int_specs.count(INT) == 1) {
             int_specs.erase(INT);
             return IntegralType::Create(token_to_intspec(*int_specs.begin()), loc);
          } else {
             error(g_filename, loc) << "too many type specifiers given" << std::endl;
             return IntegralType::Create(token_to_intspec(*int_specs.begin()), loc);
          }
          
       case 3:
          if (int_specs.count(LONG) == 2 && int_specs.count(INT) == 1) {
             return IntegralType::Create(token_to_intspec(*int_specs.begin()), loc);
          }
          
       default:
             error(g_filename, loc) << "too many type specifiers given; "
                                            << "how about I just pick one at random?"
                                            << std::endl;
             return IntegralType::Create(token_to_intspec(*int_specs.begin()), loc);
       }
    }


    /*** TYPE CHECK ***/

    void PointerType::TypeCheck(SemantEnv& env, bool allow_void) {
       pointee()->TypeCheck(env, true);
    }

    void FunctionType::TypeCheck(SemantEnv& env, bool allow_void) {
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
       return_type()->TypeCheck(env, true);
       params()->TypeCheck(env);
    }

    void StructType::TypeCheck(SemantEnv& env, bool allow_void) {
       if (membs() != nullptr) {
          /* this struct is a complete definition */
          
          membs()->TypeCheck(env);

          /* enscope struct if it isn't anonymous */
          if (struct_id() != nullptr) {
             EnscopeStruct(env);
          }
       } else {
          /* struct cannot be anonymous by C syntax */
          const StructType *type = env.structs().Lookup(struct_id());
          if (type == nullptr) {
             /* forward-declare struct */
             EnscopeStruct(env);
          } else {
             /* inherit definition of members from already-declared struct */
             membs_ = type->membs_;
          }
       }
    }

    void ArrayType::TypeCheck(SemantEnv& env, bool allow_void) {
       /* For now, assert that size parameter for arrays must always be given. */
       elem()->TypeCheck(env, false);

       if (count()->is_const()) {
          if ((int_count_ = count()->int_const()) < 0) {
             env.error()(g_filename, this) << "array has negative size" << std::endl;
             int_count_ *= -1;
          }
       } else {
          env.error()(g_filename, this) << "array size must be constant expression" << std::endl;
          int_count_ = 0;
       }
    }

    void StructType::EnscopeStruct(SemantEnv& env) {
       assert(struct_id() != nullptr); /* ensure struct isn't anonymous */

       StructType *type = env.structs().Lookup(struct_id());
       if (type == nullptr) {
          /* first mention of struct */
          env.structs().AddToScope(struct_id(), this);
       } else if (type->membs() == nullptr) {
          /* struct in table is incomplete */

          if (membs() != nullptr) {
             /* this struct is a full definition, so add members to existing enscoped struct */
             type->membs_ = membs();
          }
       } else {
          if (membs() == nullptr) {
             membs_ = type->membs();
          } else {
             if (env.structs().Probe(struct_id()) != nullptr) {
                env.error()(g_filename, this) << "redefinition of struct '" << *struct_id()
                                              << "'" << std::endl;
             }
          }
       }
    }


    void Types::TypeCheck(SemantEnv& env) {
       ASTNodeVec::TypeCheck(env);
    }
 
    void TranslationUnit::TypeCheck(SemantEnv& env) {
       Enscope(env);
       decls()->TypeCheck(env);
       Descope(env);
    }

    void ExternalDecl::TypeCheck(SemantEnv& env) {
       decl()->TypeCheck(env);
       decl()->Enscope(env);
    }

    void FunctionDef::TypeCheck(SemantEnv& env) {
       decl()->TypeCheck(env);
       Enscope(env);
       comp_stat()->TypeCheck(env, false); /* function body doesn't get new scope */
       Descope(env);
    }
    
    void Decl::TypeCheck(SemantEnv& env) {
       specs()->TypeCheck(env);
       declarator()->TypeCheck(env);
    }

    void FunctionDeclarator::TypeCheck(SemantEnv& env) {
       declarator()->TypeCheck(env);
       params()->TypeCheck(env);
    }

    void ArrayDeclarator::TypeCheck(SemantEnv& env) {
       declarator()->TypeCheck(env);
       count_expr()->TypeCheck(env);
       if (!count_expr()->is_const()) {
          env.error()(g_filename, this) << "array size expression is not constant" << std::endl;
       }
    }
    
   void CompoundStat::TypeCheck(SemantEnv& env, bool scoped) {
      decls()->TypeCheck(env);

      if (scoped) {
         env.EnterScope();
      }

      decls()->Enscope(env);      
      stats()->TypeCheck(env);

      if (scoped) {
         env.ExitScope();
      }
   }

    void ReturnStat::TypeCheck(SemantEnv& env) {
       expr()->TypeCheck(env);

       const ASTType *expr_type = expr()->type();
       Symbol *fn_sym = env.ext_env().sym();
       const FunctionType *fn_type = dynamic_cast<const FunctionType *>(env.symtab().Lookup(fn_sym));
       const ASTType *ret_type = fn_type->return_type();
       if (!ret_type->TypeCoerce(expr_type)) {
          env.error()(g_filename, this) << "value in return statement has incompatible type"
                                        << std::endl;
       }
    }

    void IfStat::TypeCheck(SemantEnv& env) {
       cond()->TypeCheck(env);
       if_body()->TypeCheck(env);
       if (else_body() != nullptr) {
          else_body()->TypeCheck(env);
       }
    }

    void WhileStat::TypeCheck(SemantEnv& env) {
       pred()->TypeCheck(env);
       body()->TypeCheck(env);
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

      /* add cast expression to coerce rhs value */
      if (!lhs_->type()->TypeEq(rhs_->type())) {
         rhs_ = CastExpr::Create(lhs_->type(), rhs_, rhs_->loc());
      }
   }

    void CallExpr::TypeCheck(SemantEnv& env) {
       fn()->TypeCheck(env);
       params()->TypeCheck(env);

       /* Verify that called expression is of function or function pointer type. */
       const FunctionType *type;
       if ((type = fn()->type()->get_callable()) == nullptr) {
          env.error()(g_filename, this) << "expression is not callable" << std::endl;
          type_ = IntegralType::Create(IntegralType::IntKind::SPEC_INT, loc());
          return;
       }

       type_ = type->return_type();
       
       /* verify that argument parameters can be coerced to function parameter types */
       if (type->params()->vec().size() != params()->vec().size()) {
          env.error()(g_filename, this) << "incorrect number of arguments given (expected"
                                        << type->params()->vec().size() << " but got "
                                        << params()->vec().size() << ")" << std::endl;
          return;
       }

       /* coerce types one-by-one */
       auto type_it = type->params()->vec().begin();
       auto type_end = type->params()->vec().end();
       auto arg_it = params()->vec().begin();
       auto arg_end = params()->vec().end();
       for (int i = 1; type_it != type_end; ++type_it, ++arg_it, ++i) {
          const ASTType *arg_type = (*arg_it)->type();
          if (!(*type_it)->TypeCoerce(arg_type)) {
             env.error()(g_filename, this) << "cannot coerce type of argument " << i << std::endl;
          }
       }
    }

    void CastExpr::TypeCheck(SemantEnv& env) {
       expr_->TypeCheck(env);
    }

    void MembExpr::TypeCheck(SemantEnv& env) {
       expr()->TypeCheck(env);
       if (expr()->type()->kind() != ASTType::Kind::TYPE_STRUCT) {
          env.error()(g_filename, this) << "member access into non-struct type" << std::endl;
       }
       const StructType *struct_type = dynamic_cast<const StructType *>(expr()->type());
       if (struct_type->membs() == nullptr) {
          env.error()(g_filename, this) << "member access into incomplete struct '"
                                        << struct_type->struct_id() << "'" << std::endl;
       } else {
          Types *membs = struct_type->membs();
          ASTType *memb_decl = membs->Lookup(memb());
          if (memb_decl == nullptr) {
             env.error()(g_filename, this) << "no member named '" << *memb() << "' in struct '"
                                           << struct_type->struct_id() << "'" << std::endl;
          } else {
             type_ = memb_decl;
          }
       }
    }

    void SizeofExpr::TypeCheck(SemantEnv& env) {
       std::visit(overloaded {
             [&](auto val) { val->TypeCheck(env); }
                }, variant_);

       type_ = IntegralType::Create(IntegralType::IntKind::SPEC_LONG_LONG, loc());
    }

    void IndexExpr::TypeCheck(SemantEnv& env) {
       using IntKind = IntegralType::IntKind;
       
       base()->TypeCheck(env);
       index()->TypeCheck(env);
       if (index()->type()->kind() != ASTType::Kind::TYPE_INTEGRAL) {
          env.error()(g_filename, this) << "cannot index type with a non-integral value"
                                        << std::endl;
       } else {
          const IntegralType *int_type = dynamic_cast<const IntegralType *>(index()->type());
          if (int_type->bytes() != z80::long_size) {
             index_ = CastExpr::Create(IntegralType::Create(IntKind::SPEC_LONG, loc()), index(),
                                       loc());
          }
       }
       type_ = base()->type()->get_containee();
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
         
         if (!(expr_->type()->kind() == ASTType::Kind::TYPE_INTEGRAL)) {
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
            bool lhs_int = lhs_->type()->kind() == ASTType::Kind::TYPE_INTEGRAL;
            bool rhs_int = rhs_->type()->kind() == ASTType::Kind::TYPE_INTEGRAL;

            if (lhs_int && rhs_int) {
               type_ = dynamic_cast<const IntegralType *>
                  (lhs_->type())->Max(dynamic_cast<const IntegralType *>(rhs_->type()));

               /* make implicit casts explicit */
               if (!lhs_->type()->TypeEq(type_)) {
                  lhs_ = CastExpr::Create(type_, lhs_, lhs_->loc());
               } else if (!rhs_->type()->TypeEq(type_)) {
                  rhs_ = CastExpr::Create(type_, rhs_, rhs_->loc());
               }
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

            if (is_logical()) {
               type_ = IntegralType::Create(IntegralType::IntKind::SPEC_CHAR, loc());
            }
         }
      }
   }

   void LiteralExpr::TypeCheck(SemantEnv& env) {
      type_ = IntegralType::Create(IntegralType::IntKind::SPEC_LONG_LONG, loc());
   }

   void StringExpr::TypeCheck(SemantEnv& env) {
      type_ = PointerType::Create(1, IntegralType::Create(IntegralType::IntKind::SPEC_CHAR, loc()),
                                  loc());
   }

   void IdentifierExpr::TypeCheck(SemantEnv& env) {
      if ((type_ = env.symtab().Lookup(id()->id())) == nullptr) {
         env.error()(g_filename, this) << "use of undeclared identifier '" << *id()->id()
                                       << "'" << std::endl;
         type_ = IntegralType::Create(IntegralType::IntKind::SPEC_INT, loc());         
      }
   }

    void NoExpr::TypeCheck(SemantEnv& env) {
       /* set type */
       type_ = VoidType::Create(loc());
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

    /*** TYPE EQUALITY ***/
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

    bool StructType::TypeEq(const ASTType *other) const {
       const StructType *st_other = dynamic_cast<const StructType *>(other);
       if (st_other == nullptr) {
          return false;
       } else {
          return this->struct_id() != nullptr && st_other->struct_id() != nullptr &&
             this->struct_id() == st_other->struct_id();
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
   bool PointerType::TypeCoerce(const ASTType *from) const {
      /* -1. Check if types are equal. */
      if (this->TypeEq(from)) {
         return true;
      }

      /* check if `from' is an array. */
      const ArrayType *from_arr = dynamic_cast<const ArrayType *>(from);
      if (from_arr != nullptr) {
         /* ensure base elements are of same type */
         return from_arr->TypeEq(pointee());
      }
      
      /* 0. Verify `from' is a pointer. */
      const PointerType *from_ptr = dynamic_cast<const PointerType *>(from);
      if (from_ptr == nullptr) {
         return false; /* can never implicitly cast from non-pointer type to pointer type */
      }
      
      /* 1. Check if coercing to 'void *'. */
      if (pointee()->kind() == ASTType::Kind::TYPE_VOID) {
         return true;
      }

      return false; /* cannot coerce since pointers are non-identical and `to' is not void ptr */
   }
   
   bool FunctionType::TypeCoerce(const ASTType *from) const {
      return TypeEq(from);
   }

    bool StructType::TypeCoerce(const ASTType *from) const {
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

    ASTType *ExternalDecl::Type() const {
       return decl();
    }
    
   ASTType *BasicDeclarator::Type(ASTType *type) const {
      return type;
   }

   /* This is so confusing. */
   ASTType *PointerDeclarator::Type(ASTType *type) const {
      switch (declarator()->kind()) {
      case Kind::DECLARATOR_BASIC:
         /* basic pointer */
         return PointerType::Create(depth(), declarator()->Type(type), type->sym(), loc());
         
      case Kind::DECLARATOR_POINTER:
         /* this shouldn't happen */
         abort();
         
      case Kind::DECLARATOR_FUNCTION:
         /* tricky: pointer is actualy part of the return value type */
         return declarator()->Type(PointerType::Create(depth(), type, type->sym(), loc()));

      case Kind::DECLARATOR_ARRAY:
         /* pointer is part of the element type. */
         return declarator()->Type(PointerType::Create(depth(), type, type->sym(), loc()));
      }
   }

   ASTType *FunctionDeclarator::Type(ASTType *type) const {
      /* NOTE: _type_ represents the (possibly partial) return value of this function. */
      Types *param_types = params();

      /* WHOA -- if this works, it's beautiful. */
      switch (declarator()->kind()) {
      case Kind::DECLARATOR_BASIC:
         return FunctionType::Create(type, param_types, type->sym(), loc());
         
      case Kind::DECLARATOR_POINTER:
         /* Actually a function pointer -- but primarily a pointer. */
         return declarator()->Type(FunctionType::Create(type, param_types, type->sym(), loc()));
         
      case Kind::DECLARATOR_FUNCTION:
         /* This function is actually the RETURN TYPE of another function declared
          * _declarator()_. */
         return declarator()->Type(FunctionType::Create(type, param_types, type->sym(), loc()));

      case Kind::DECLARATOR_ARRAY:
         /* Who in their right mind would try to declare an array of functions?! */
         const char *errmsgs[2] = {"who in their right mind would try to declare" \
                                   " an array of functions?",
                                   "please consider, well, uh, NOT declaring an array of " \
                                   "functions?"};
         g_semant_error(g_filename, this) << errmsgs[(intptr_t) this % 7 % 2] << std::endl;
         return declarator()->Type(PointerType::Create(1, type, type->sym(), loc()));
      }
   }

    ASTType *ArrayDeclarator::Type(ASTType *type) const {
       /* NOTE: _type_ represents the (possibly partial) element type of this array. */
       switch (declarator()->kind()) {
       case Kind::DECLARATOR_BASIC:
          return ArrayType::Create(type, count_expr(), type->sym(), loc());
          
       case Kind::DECLARATOR_POINTER:
          /* Actually a pointer to an array. */
          return declarator()->Type(ArrayType::Create(type, count_expr(), type->sym(), loc()));

       case Kind::DECLARATOR_FUNCTION:
          /* Functions can't return arrays. */
          g_semant_error(g_filename, this) << "functions can't return arrays" << std::endl;
          return declarator()->Type(PointerType::Create(1, type, type->sym(), loc()));

       case Kind::DECLARATOR_ARRAY:
          /* Swap array order. */
          return declarator()->Type(ArrayType::Create(type, count_expr(), type->sym(), loc()));
       }
    }

   ASTType *Decl::Type() const {
      ASTType *init_type = specs();
      init_type->set_sym(sym());
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
    ASTType *PointerType::Address() {
       return PointerType::Create(depth() + 1, pointee(), loc());
    }
    
    ASTType *FunctionType::Address() {
       return this;
    }

    ASTType *StructType::Address() {
       return PointerType::Create(1, this, loc());
    }

    ASTType *ArrayType::Address() {
       return PointerType::Create(1, this, loc());
    }
          
    
    /*** DEREFERENCE TYPE ***/
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

    ASTType *StructType::Dereference(SemantEnv *env) {
       if (env) {
          env->error()(g_filename, this)
             << "do you really think you can dereference a struct?" << std::endl;
       }
       return nullptr;
    }

    /*** ENSCOPE ***/
    void TranslationUnit::Enscope(SemantEnv& env) const {
       env.EnterScope();
    }

    void TranslationUnit::Descope(SemantEnv& env) const {
       env.ExitScope();
     }

     void ASTType::Enscope(SemantEnv& env) { 
        /* check for previous declarations in scope */
        if (sym() == nullptr) {
           // env.error()(g_filename, this) << "declaration is missing identifier" << std::endl;
           /* NOTE: this actually shouldn't be an error. Some compilers generate it as a warning. */
           return;
        }
     
        if (env.symtab().Probe(sym()) != nullptr) {
           /* ERROR: symbol already defined in this scope. */
           env.error()(g_filename, this) << "redefinition of '" << *sym() << "'" << std::endl;
           return;
        }
     
        /* add symbol to scope */
        env.symtab().AddToScope(sym(), this);
     }

     void ExternalDecl::Enscope(SemantEnv& env) const {
        decl()->Enscope(env);
        env.ext_env().Enter(decl()->sym());
     }

     void ExternalDecl::Descope(SemantEnv& env) const {
        env.ext_env().Exit();
     }

     void FunctionDef::Enscope(SemantEnv& env) const {
        /* enscope function symbol */
        ExternalDecl::Enscope(env);

        /* add new scope */
        env.EnterScope();

       /* enscope parameters */
       FunctionType *type;
       if ((type = dynamic_cast<FunctionType *>(Type())) == nullptr) {
          env.error()(g_filename, this) << "function definition missing parameters" << std::endl;
          return;
       }

       Types *params = type->params();
       std::for_each(params->vec().begin(), params->vec().end(),
                     [&](ASTType *param) {
                        param->Enscope(env);
                     });
    }

    void FunctionDef::Descope(SemantEnv& env) const {
       env.ExitScope();
    }


}
