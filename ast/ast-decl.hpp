#ifndef __AST_HPP
#error "include ast.hpp, not ast/ast-decl.h directly"
#endif

#ifndef __AST_DECL_HPP
#define __AST_DECL_HPP

#include "ast/ast-base.hpp"

namespace zc {

   class ASTType;
   class Types;

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

   class Decl: public ASTNode {
   public:
      ASTType *specs() const { return specs_; }
      ASTDeclarator *declarator() const { return declarator_; }
      Identifier *id() const;
      Symbol *sym() const;

      template <typename... Args>
      static Decl *Create(Args... args) {
         return new Decl(args...);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "Decl"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;
      virtual void DumpType(std::ostream& os) const override;

      /* Semantic Analysis */
      
      /**
       * Perform semantic analysis on a declaration.
       * @param env semantic environment
       * @param enscope whether to declare in scope.
       */
      void TypeCheck(SemantEnv& env);

      /** Convert declaration to type.
       */
      ASTType *Type() const;

      void JoinPointers();

      /* Code Generation */
      void CodeGen(CgenEnv& env);
      void FrameGen(StackFrame& frame) const;

   protected:
      ASTType *specs_;
      ASTDeclarator *declarator_;
      
      Decl(ASTType *specs, ASTDeclarator *declarator, const SourceLoc& loc):
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


   /* NOTE: Abstract class. */
   class TypeSpec: public ASTNode {
   public:
      enum class Kind {SPEC_VOID, SPEC_INTEGRAL, SPEC_STRUCT};
      virtual Kind kind() const = 0;

      virtual bool Eq(const TypeSpec *other) const = 0;

      virtual void TypeCheck(SemantEnv& env, bool allow_void) = 0;

   protected:
      template <typename... Args>
      TypeSpec(Args... args): ASTNode(args...) {}
   };

   class VoidSpec: public TypeSpec {
   public:
      virtual Kind kind() const override { return Kind::SPEC_VOID; }

      virtual void DumpNode(std::ostream& os) const override { os << "VoidSpec VOID"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {}

      virtual bool Eq(const TypeSpec *other) const override;

      virtual void TypeCheck(SemantEnv& env, bool allow_void) override {}
      
      template <typename... Args>
      static VoidSpec *Create(Args... args) { return new VoidSpec(args...); }
   private:
      template <typename... Args>
      VoidSpec(Args... args): TypeSpec(args...) {}
   };

   class IntegralSpec: public TypeSpec {
   public:
      virtual Kind kind() const override { return Kind::SPEC_INTEGRAL; }
      enum class IntKind {SPEC_CHAR, SPEC_SHORT, SPEC_INT, SPEC_LONG, SPEC_LONG_LONG};
      IntKind int_kind() const { return int_kind_; }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {}

      virtual bool Eq(const TypeSpec *other) const override;      

      virtual void TypeCheck(SemantEnv& env, bool allow_void) override {}
      
      IntegralSpec *Max(const IntegralSpec *other) const;

      template <typename... Args>
      static IntegralSpec *Create(Args... args) { return new IntegralSpec(args...); }
      
   protected:
      IntKind int_kind_;

      template <typename... Args>
      IntegralSpec(IntKind int_kind, Args... args): TypeSpec(args...), int_kind_(int_kind) {}
   };

   class StructSpec: public TypeSpec {
   public:
      virtual Kind kind() const override { return Kind::SPEC_STRUCT; }

      Identifier *id() const { return id_; }
      Decls *membs() const { return membs_; }
      Symbol *sym() const;

      virtual bool Eq(const TypeSpec *other) const override;
      
      virtual void DumpNode(std::ostream& os) const override { os << "StructSpec STRUCT"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;

      virtual void TypeCheck(SemantEnv& env, bool allow_void) override;
      
      template <typename... Args>
      static StructSpec *Create(Args... args) { return new StructSpec(args...); }
      
   private:
      Identifier *id_;
      Decls *membs_;

      template <typename... Args>
      StructSpec(Identifier *id, Decls *membs, Args... args):
         TypeSpec(args...), id_(id), membs_(membs) {}
   };
      

   class TypeSpecs: public ASTNodeVec<TypeSpec> {
   public:
      static TypeSpecs *Create(const SourceLoc& loc) { return new TypeSpecs(loc); }
      
      TypeSpec *TypeCombine() const;
      TypeSpec *TypeCombine(SemantEnv *env) const;
      
      void TypeCheck(SemantEnv& env);
      
   protected:
      TypeSpecs(const SourceLoc& loc): ASTNodeVec<TypeSpec>(loc) {}
   };

#if 0   
   /**
    * Collection of declaration specifiers.
    * NOTE: This is ephemeral; it is converted to a type during parsing.
    */
   class DeclSpecs: public ASTNode {
   public:
      typedef std::vector<int> Tokens;
      Tokens& int_specs() { return int_specs_; }
      
      
      TypeSpecs *type_specs() const { return type_specs_; }
      TypeSpec *type_spec() const { return type_specs()->TypeCombine(); }

      static DeclSpecs *Create(const SourceLoc& loc) { return new DeclSpecs(loc); }

      virtual void DumpNode(std::ostream& os) const override { os << "DeclSpecs"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;

      void TypeCheck(SemantEnv& env);
      bool TypeEq(const DeclSpecs *other) const;
      bool TypeCoerce(const DeclSpecs *from) const;

   protected:
      Tokens int_specs_;
      
      DeclSpecs(const SourceLoc& loc): ASTNode(loc), type_specs_(TypeSpecs::Create(loc)) {}
   };
#endif

   
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


   std::ostream& operator<<(std::ostream& os, IntegralSpec::IntKind kind);

}

#endif
