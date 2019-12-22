#ifndef __AST_HPP
#error "include ast.hpp, not ast/ast-decl.h directly"
#endif

#ifndef __AST_DECL_HPP
#define __AST_DECL_HPP

#include "ast/ast-base.hpp"

namespace zc {

   /* NOTE: ExternalDecl is abstract. */
   class ExternalDecl: virtual public ASTNode {
   public:
   protected:
      ExternalDecl(const SourceLoc& loc): ASTNode(loc) {}
   };

   class FunctionDef: public ExternalDecl {
   public:
      Decl *decl() const { return decl_; }
      CompoundStat *comp_stat() const { return comp_stat_; }

      static FunctionDef *Create(Decl *decl, CompoundStat *comp_stat, const SourceLoc& loc) {
         return new FunctionDef(decl, comp_stat, loc);
      }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level) const override;

      virtual void TypeCheck(SemantEnv& env, bool scoped = true) override;
      
   protected:
      Decl *decl_;
      CompoundStat *comp_stat_;

   FunctionDef(Decl *decl, CompoundStat *comp_stat, const SourceLoc& loc):
      ASTNode(loc), ExternalDecl(loc), decl_(decl), comp_stat_(comp_stat) {}
      
   };

   class Decl: public ASTNode {
   public:
      DeclSpecs *specs() const { return specs_; }
      ASTDeclarator *declarator() const { return declarator_; }
      Identifier *id() const;
      
      static Decl *Create(DeclSpecs *specs, ASTDeclarator *declarator, const SourceLoc& loc) {
         return new Decl(specs, declarator, loc);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "Decl"; }
      virtual void DumpChildren(std::ostream& os, int level) const override;

      virtual void TypeCheck(SemantEnv& env, bool scoped = true) override;

      bool TypeCompat(const Decl *other) const;

      /** Obtain dereferenced type.
       * @return dereferenced type on success; nullptr if not dereferencable.
       */
      Decl *Dereference();

      /** Obtain address type.
       * @return address type
       */
      Decl *Address();

   protected:
      DeclSpecs *specs_;
      ASTDeclarator *declarator_;
      
      Decl(DeclSpecs *specs, ASTDeclarator *declarator, const SourceLoc& loc):
         ASTNode(loc), specs_(specs), declarator_(declarator) {}
   };

   const char Decls_s[] = "Decls";
   class Decls: public virtual ASTNodeVec<Decl,Decls_s>, public virtual ExternalDecl {
   public:
      static Decls *Create(const SourceLoc& loc) { return new Decls(loc); }

      virtual void DumpNode(std::ostream& os) const override { os << "Decls"; }

      bool TypeCompat(const Decls *other) const;
      
   protected:
      Decls(const SourceLoc& loc): ASTNode(loc), ASTNodeVec<Decl,Decls_s>(loc), ExternalDecl(loc) {}
   };


   const char constexpr TypeSpecs_s[] = "TypeSpecs";   
   class TypeSpecs: public ASTSpecs<TypeSpec,TypeSpecs_s> {
   public:
      static TypeSpecs *Create(const SourceLoc& loc) { return new TypeSpecs(loc); }

      virtual void TypeCheck(SemantEnv& env, bool scoped = true) override;

      bool TypeCompat(const TypeSpecs *other) const;
      
   protected:

      TypeSpecs(const SourceLoc& loc): ASTSpecs<TypeSpec,TypeSpecs_s>(loc) {}
   };


   class DeclSpecs: public ASTNode {
   public:
      TypeSpecs *type_specs() const { return type_specs_; }

      static DeclSpecs *Create(const SourceLoc& loc) { return new DeclSpecs(loc); }

      virtual void DumpNode(std::ostream& os) const override { os << "DeclSpecs"; }
      virtual void DumpChildren(std::ostream& os, int level) const override;

      virtual void TypeCheck(SemantEnv& env, bool scoped = true) override {
         type_specs()->TypeCheck(env);
      }

      bool TypeCompat(const DeclSpecs *other) const;

   protected:
      TypeSpecs *type_specs_;
      
      DeclSpecs(const SourceLoc& loc): ASTNode(loc), type_specs_(TypeSpecs::Create(loc)) {}
   };

   
   std::ostream& operator<< (std::ostream& os, const TypeSpec& spec);

   class ASTDeclarator: public ASTNode {
   public:
      virtual Identifier *id() const = 0;

      virtual bool TypeCompat(const ASTDeclarator *other) const = 0;
      virtual ASTDeclarator *Dereference() = 0;
      virtual ASTDeclarator *Address() = 0;
      
   protected:
      ASTDeclarator(const SourceLoc& loc): ASTNode(loc) {}
   };

   class PointerDeclarator: public ASTDeclarator {
   public:
      int depth() const { return depth_; }
      ASTDeclarator *declarator() const { return declarator_; }
      virtual Identifier *id() const override { return declarator_->id(); }
      
      static PointerDeclarator *Create(int depth, ASTDeclarator *declarator, const SourceLoc& loc)
      { return new PointerDeclarator(depth, declarator, loc); }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level) const override {
         declarator_->Dump(os, level);
      }

      virtual void TypeCheck(SemantEnv& env, bool scoped = true) override {
         declarator()->TypeCheck(env);
      }
      virtual bool TypeCompat(const ASTDeclarator *other) const override;
      virtual ASTDeclarator *Dereference() override;
      virtual ASTDeclarator *Address() override;
      
   protected:
      int depth_;
      ASTDeclarator *declarator_;
      PointerDeclarator(int depth, ASTDeclarator *declarator, const SourceLoc& loc):
         ASTDeclarator(loc), depth_(depth), declarator_(declarator) {}
   };

   class BasicDeclarator: public ASTDeclarator {
   public:
      virtual Identifier *id() const override { return id_; }

      static BasicDeclarator *Create(Identifier *id, const SourceLoc& loc) {
         return new BasicDeclarator(id, loc);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "BasicDeclarator"; }
      virtual void DumpChildren(std::ostream& os, int level) const override;

      virtual void TypeCheck(SemantEnv& env, bool scoped = true) override {
         /* redefinition checked by Decl */
      }
      virtual bool TypeCompat(const ASTDeclarator *other) const override;
      virtual ASTDeclarator *Dereference() override { return nullptr; }
      virtual ASTDeclarator *Address() override;
      
   protected:
      Identifier *id_;
      BasicDeclarator(Identifier *id, const SourceLoc& loc):
         ASTDeclarator(loc), id_(id) {}
   };

   class FunctionDeclarator: public ASTDeclarator {
   public:
      ASTDeclarator *declarator() const { return declarator_; }
      Decls *params() const { return params_; }
      virtual Identifier *id() const override { return declarator_->id(); }
      
      static FunctionDeclarator *Create(ASTDeclarator *declarator, Decls *params,
                                        const SourceLoc& loc) {
         return new FunctionDeclarator(declarator, params, loc);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "FunctionDeclarator"; }
      virtual void DumpChildren(std::ostream& os, int level) const override;

      virtual void TypeCheck(SemantEnv& env, bool scoped = true) override;
      bool TypeCompat(const ASTDeclarator *other) const override;
      virtual ASTDeclarator *Dereference() override { return this; }
      virtual ASTDeclarator *Address() override { return this; }
      
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
      virtual void DumpChildren(std::ostream& os, int level) const override { /* no children */ }

      virtual void TypeCheck(SemantEnv& env, bool scoped = true) override {}
      
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
