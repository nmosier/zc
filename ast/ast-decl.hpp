#ifndef __AST_HPP
#error "include ast.hpp, not ast/ast-decl.h directly"
#endif

#ifndef __AST_DECL_HPP
#define __AST_DECL_HPP

namespace zc {

   class ExternalDecl: public ASTNode, public ASTVariantFeature<FunctionDef, Decls> {
   public:
      static ExternalDecl *Create(const Variant& variant, const SourceLoc& loc)
      { return new ExternalDecl(variant, loc); }
      
   protected:
   ExternalDecl(const Variant& variant, const SourceLoc& loc): ASTNode(loc), ASTVariantFeature(variant) {}
   };

   class FunctionDef: public ASTNode {
   public:
      DeclSpecs *specs() const { return specs_; }
      ASTDeclarator *declarator() const { return declarator_; }
      CompoundStat *comp_stat() const { return comp_stat_; }

      static FunctionDef *Create(DeclSpecs *specs, ASTDeclarator *declarator, CompoundStat *comp_stat,
                                 const SourceLoc& loc)
      { return new FunctionDef(specs, declarator, comp_stat, loc); }
      
   protected:
      DeclSpecs *specs_;
      ASTDeclarator *declarator_;
      CompoundStat *comp_stat_;

   FunctionDef(DeclSpecs *specs, ASTDeclarator *declarator, CompoundStat *comp_stat,
               const SourceLoc& loc): ASTNode(loc), specs_(specs), declarator_(declarator),
         comp_stat_(comp_stat) {}
      
   };

   
   class Decl: public ASTNode {
   public:
      DeclSpecs *specs() const { return specs_; }
      
      static Decl *Create(DeclSpecs *specs, ASTDeclarator *declarator, const SourceLoc& loc)
      { return new Decl(specs, declarator, loc); }
      
   protected:
      DeclSpecs *specs_;
      ASTDeclarator *declarator_;

      Decl(DeclSpecs *specs, ASTDeclarator *declarator, const SourceLoc& loc):
         ASTNode(loc), specs_(specs), declarator_(declarator) {}
   };
   
   class DeclSpec: public ASTNode {
   public:
   protected:
      DeclSpec(const SourceLoc& loc): ASTNode(loc) {}
   };

   class TypeSpec: public DeclSpec {
   public:
      enum Kind { TYPE_VOID, TYPE_CHAR, TYPE_SHORT, TYPE_INT, TYPE_LONG };
      Kind kind() const { return kind_; }
      
      static TypeSpec *Create(enum Kind kind, const SourceLoc& loc) { return new TypeSpec(kind, loc); }
      
   protected:
      Kind kind_;
      
      TypeSpec(Kind kind, const SourceLoc& loc): DeclSpec(loc), kind_(kind) {}
   };

   class ASTDeclarator: public ASTNode {
   public:
   protected:
      ASTDeclarator(const SourceLoc& loc): ASTNode(loc) {}
   };

   class PointerDeclarator: public ASTDeclarator {
   public:
      int depth() const { return depth_; }
      ASTDeclarator *declarator() const { return declarator_; }
      
      static PointerDeclarator *Create(int depth, ASTDeclarator *declarator, const SourceLoc& loc)
      { return new PointerDeclarator(depth, declarator, loc); }
   protected:
      int depth_;
      ASTDeclarator *declarator_;
      PointerDeclarator(int depth, ASTDeclarator *declarator, const SourceLoc& loc):
         ASTDeclarator(loc), depth_(depth), declarator_(declarator) {}
   };

   class BasicDeclarator: public ASTDeclarator {
   public:
      Identifier *id() const { return id_; }
      static BasicDeclarator *Create(Identifier *id, const SourceLoc& loc)
      { return new BasicDeclarator(id, loc); }
   protected:
      Identifier *id_;
      BasicDeclarator(Identifier *id, const SourceLoc& loc):
         ASTDeclarator(loc), id_(id) {}
   };

   class FunctionDeclarator: public ASTDeclarator {
   public:
      ASTDeclarator *declarator() const { return declarator_; }
      Decls *params() const { return params_; }
      static FunctionDeclarator *Create(ASTDeclarator *declarator, Decls *params,
                                        const SourceLoc& loc)
      { return new FunctionDeclarator(declarator, params, loc); }
   protected:
      ASTDeclarator *declarator_;
      Decls *params_;
      FunctionDeclarator(ASTDeclarator *declarator, Decls *params, const SourceLoc& loc):
         ASTDeclarator(loc), declarator_(declarator), params_(params) {}
   };

}

#endif
