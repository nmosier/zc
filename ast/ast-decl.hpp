#ifndef __AST_HPP
#error "include ast.hpp, not ast/ast-decl.h directly"
#endif

#ifndef __AST_DECL_HPP
#define __AST_DECL_HPP

#include "ast/ast-base.hpp"
#include "ast/ast-type.hpp"

namespace zc {

  class ExternalDecl: public ASTNode {
   public:
     Decl *decl() const { return decl_; }
     Symbol *sym() const;
     
     template <typename... Args>
     static ExternalDecl *Create(Args... args) {
         return new ExternalDecl(args...);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "ExternalDecl"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;
      
      virtual void TypeCheck(SemantEnv& env);
      ASTType *Type() const;
      virtual void Enscope(SemantEnv& env) const;
      virtual void Descope(SemantEnv& env) const;

      virtual void CodeGen(CgenEnv& env);
      
   protected:
      Decl *decl_;

      template <typename... Args>
      ExternalDecl(Decl *decl, Args... args): ASTNode(args...), decl_(decl) {}
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
      
   protected:
      CompoundStat *comp_stat_;

      template <typename... Args>
      FunctionDef(CompoundStat *comp_stat, Args... args):
         ExternalDecl(args...), comp_stat_(comp_stat) {}
      
   };

   class Decl: public ASTNode {
   public:
      DeclSpecs *specs() const { return specs_; }
      ASTDeclarator *declarator() const { return declarator_; }
      Identifier *id() const;
      Symbol *sym() const;
      
      static Decl *Create(DeclSpecs *specs, ASTDeclarator *declarator, const SourceLoc& loc) {
         return new Decl(specs, declarator, loc);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "Decl"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;
      virtual void DumpType(std::ostream& os) const override;

      /**
       * Perform semantic analysis on a declaration.
       * @param env semantic environment
       * @param enscope whether to declare in scope.
       */
      void TypeCheck(SemantEnv& env);
      void Enscope(SemantEnv& env) const;

      /** Convert declaration to type.
       */
      ASTType *Type() const;

      void JoinPointers();

      void CodeGen(CgenEnv& env);

   protected:
      DeclSpecs *specs_;
      ASTDeclarator *declarator_;
      
      Decl(DeclSpecs *specs, ASTDeclarator *declarator, const SourceLoc& loc):
         ASTNode(loc), specs_(specs), declarator_(declarator) {}
   };

   class Decls: public ASTNodeVec<Decl> {
   public:
      static Decls *Create(const SourceLoc& loc) { return new Decls(loc); }

      virtual void DumpNode(std::ostream& os) const override { os << "Decls"; }

      void TypeCheck(SemantEnv& env) {
         ASTNodeVec<Decl>::TypeCheck(env);
      }
      bool TypeEq(const Decls *other) const;
      bool TypeCoerce(const Decls *from) const;
      Types *Type() const;
      void Enscope(SemantEnv& env) const;

      void JoinPointers();
      
   protected:
      template <typename... Args> Decls(Args... args): ASTNodeVec<Decl>(args...) {}
   };


   class TypeSpecs: public ASTSpecs<TypeSpec> {
   public:
      static TypeSpecs *Create(const SourceLoc& loc) { return new TypeSpecs(loc); }
      
      TypeSpec TypeCombine() const;
      TypeSpec TypeCombine(SemantEnv *env) const;
      
      void TypeCheck(SemantEnv& env);
      
   protected:
      TypeSpecs(const SourceLoc& loc): ASTSpecs<TypeSpec>(loc) {}
   };


   class DeclSpecs: public ASTNode {
   public:
      TypeSpecs *type_specs() const { return type_specs_; }
      TypeSpec type_spec() const { return type_specs()->TypeCombine(); }

      static DeclSpecs *Create(const SourceLoc& loc) { return new DeclSpecs(loc); }

      virtual void DumpNode(std::ostream& os) const override { os << "DeclSpecs"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;

      void TypeCheck(SemantEnv& env);
      bool TypeEq(const DeclSpecs *other) const;
      bool TypeCoerce(const DeclSpecs *from) const;

   protected:
      TypeSpecs *type_specs_;
      
      DeclSpecs(const SourceLoc& loc): ASTNode(loc), type_specs_(TypeSpecs::Create(loc)) {}
   };

   
   std::ostream& operator<< (std::ostream& os, TypeSpec spec);

   class ASTDeclarator: public ASTNode {
   public:
      /** The fundamental kind of declarator. */
      enum class Kind {DECLARATOR_POINTER,   /*!< pointer */
                       DECLARATOR_BASIC,     /*!< direct value */
                       DECLARATOR_FUNCTION}; /*!< function */
      virtual Identifier *id() const = 0;
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
      Decls *params() const { return params_; }
      virtual Identifier *id() const override { return declarator_->id(); }
      virtual Kind kind() const override { return Kind::DECLARATOR_FUNCTION; }
      
      static FunctionDeclarator *Create(ASTDeclarator *declarator, Decls *params,
                                        const SourceLoc& loc) {
         return new FunctionDeclarator(declarator, params, loc);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "FunctionDeclarator"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;

      virtual void TypeCheck(SemantEnv& env) override;
      
      virtual ASTType *Type(ASTType *init_type) const override;
      
      virtual void JoinPointers() override;
      
   protected:
      ASTDeclarator *declarator_;
      Decls *params_;

      FunctionDeclarator(ASTDeclarator *declarator, Decls *params, const SourceLoc& loc):
         ASTDeclarator(loc), declarator_(declarator), params_(params) {}
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
