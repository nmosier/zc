#ifndef __AST_HPP
#error "include ast.hpp, not ast/ast-decl.h directly"
#endif

#ifndef __AST_DECL_HPP
#define __AST_DECL_HPP

#include "ast/ast-base.hpp"

namespace zc {

   class ASTType;

   class ExternalDecl: public ASTNode {
   public:
      ASTType *decl() const { return decl_; }
      Symbol *sym() const;
      
      template <typename... Args>
      static ExternalDecl *Create(Args... args) {
         return new ExternalDecl(args...);
      }
      
      virtual void DumpNode(std::ostream& os) const override { os << "ExternalDecl"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;
      
      /* Semantic Analysis */
      virtual void TypeCheck(SemantEnv& env);
      ASTType *Type() const;
      virtual void Enscope(SemantEnv& env) const;
      virtual void Descope(SemantEnv& env) const;

     /* Code Generation */
      virtual void CodeGen(CgenEnv& env);
      
   protected:
      ASTType *decl_;

      template <typename... Args>
      ExternalDecl(ASTType *decl, Args... args): ASTNode(args...), decl_(decl) {}
   };
   
   template <> const char *ExternalDecls::name() const;

   class FunctionDef: public ExternalDecl {
   public:
      CompoundStat *comp_stat() const { return comp_stat_; }

      template <typename... Args>
      static FunctionDef *Create(Args... args) {
         return new FunctionDef(args...);
      }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;

      virtual void TypeCheck(SemantEnv& env) override;
      virtual void Enscope(SemantEnv& env) const override;
      virtual void Descope(SemantEnv& env) const override;

      virtual void CodeGen(CgenEnv& env) override;
      void FrameGen(StackFrame& frame) const;
      
   protected:
      CompoundStat *comp_stat_;

      template <typename... Args>
      FunctionDef(CompoundStat *comp_stat, Args... args):
         ExternalDecl(args...), comp_stat_(comp_stat) {}
      
   };
   
   class ASTDeclarator: public ASTNode {
   public:
      /** The fundamental kind of declarator. */
      enum class Kind {DECLARATOR_POINTER,
                       DECLARATOR_BASIC, 
                       DECLARATOR_FUNCTION,
                       DECLARATOR_ARRAY};
      virtual Identifier *id() const = 0;
      Symbol *sym() const;
      virtual Kind kind() const = 0;

      virtual ASTType *Type(ASTType *init_type) const = 0;

      /**
       * Perform semantic analysis on declarator.
       * @param env semantic environment
       * @param level abstaction level. @see Decl::TypeCheck(Semantic&, int)
       */
      virtual void TypeCheck(SemantEnv& env) = 0;

      /** Unwrap a type during Decl -> Type conversion. */
      virtual void JoinPointers() = 0;      
      
   protected:
      
      ASTDeclarator(const SourceLoc& loc): ASTNode(loc) {}
   };

   
   class BasicDeclarator: public ASTDeclarator {
   public:
      virtual Identifier *id() const override { return id_; }
      virtual Kind kind() const override { return Kind::DECLARATOR_BASIC; }

      static BasicDeclarator *Create(Identifier *id, const SourceLoc& loc) {
         return new BasicDeclarator(id, loc);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "BasicDeclarator"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;

      virtual void TypeCheck(SemantEnv& env) override {}

      virtual ASTType *Type(ASTType *init_type) const override;

      virtual void JoinPointers() override { /* base case */ }
      
   protected:
      Identifier *id_;

      
      BasicDeclarator(Identifier *id, const SourceLoc& loc):
         ASTDeclarator(loc), id_(id) {}
   };


   class PointerDeclarator: public ASTDeclarator {
   public:
      int depth() const { return depth_; }
      ASTDeclarator *declarator() const { return declarator_; }
      virtual Identifier *id() const override { return declarator_->id(); }
      virtual Kind kind() const override { return Kind::DECLARATOR_POINTER; }
      
      static PointerDeclarator *Create(int depth, ASTDeclarator *declarator, const SourceLoc& loc)
      { return new PointerDeclarator(depth, declarator, loc); }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {
         declarator_->Dump(os, level, with_types);
      }

      virtual void TypeCheck(SemantEnv& env) override {
         declarator()->TypeCheck(env);
      }
      
      virtual ASTType *Type(ASTType *init_type) const override;
      
      virtual void JoinPointers() override;

   protected:
      int depth_;
      ASTDeclarator *declarator_;

      PointerDeclarator(int depth, ASTDeclarator *declarator, const SourceLoc& loc):
         ASTDeclarator(loc), depth_(depth), declarator_(declarator) {}
   };

   class FunctionDeclarator: public ASTDeclarator {
   public:
      ASTDeclarator *declarator() const { return declarator_; }
      Types *params() const { return params_; }
      virtual Identifier *id() const override { return declarator_->id(); }
      virtual Kind kind() const override { return Kind::DECLARATOR_FUNCTION; }

      template <typename... Args>
      static FunctionDeclarator *Create(Args... args) {
         return new FunctionDeclarator(args...);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "FunctionDeclarator"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;

      virtual void TypeCheck(SemantEnv& env) override;
      
      virtual ASTType *Type(ASTType *init_type) const override;
      
      virtual void JoinPointers() override;
      
   protected:
      ASTDeclarator *declarator_;
      Types *params_;

      FunctionDeclarator(ASTDeclarator *declarator, Types *params, const SourceLoc& loc):
         ASTDeclarator(loc), declarator_(declarator), params_(params) {}
   };

   class ArrayDeclarator: public ASTDeclarator {
   public:
      ASTDeclarator *declarator() const { return declarator_; }
      ASTExpr *count_expr() const { return count_expr_; }
      virtual Identifier *id() const override { return declarator_->id(); }
      virtual Kind kind() const override { return Kind::DECLARATOR_ARRAY; }

      template <typename... Args>
      static ArrayDeclarator *Create(Args... args) { return new ArrayDeclarator(args...); }

      virtual void DumpNode(std::ostream& os) const override { os << "ArrayDeclarator"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;
      virtual void TypeCheck(SemantEnv& env) override;
      virtual ASTType *Type(ASTType *init_type) const override;
      virtual void JoinPointers() override { declarator()->JoinPointers(); }

      
   protected:
      ASTDeclarator *declarator_;
      ASTExpr *count_expr_;

      template <typename... Args>
      ArrayDeclarator(ASTDeclarator *declarator, ASTExpr *count_expr, Args... args):
         ASTDeclarator(args...), declarator_(declarator), count_expr_(count_expr) {}
   };


   class Identifier: public ASTNode {
   public:
      Symbol *id() const { return id_; }
      
      static Identifier *Create(const std::string& id, SourceLoc& loc) { return new Identifier(id, loc); }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {
         /* no children */
      }

      void TypeCheck(SemantEnv& env) {}
      
   protected:
      Symbol *id_;
      
      Identifier(const std::string& id, SourceLoc& loc): ASTNode(loc), id_(nullptr) {
         if (g_id_tab.find(id) == g_id_tab.end()) {
            g_id_tab[id] = new std::string(id);
         }
         id_ = g_id_tab[id];
      }
   };




}

#endif
