#ifndef __AST_HPP
#error "include ast.hpp, not ast/ast-decl.h directly"
#endif

#ifndef __AST_DECL_HPP
#define __AST_DECL_HPP

namespace zc {

   class ExternalDecl: public ASTNode, public ASTVariantFeature<FunctionDef, Decl> {
   public:
      static ExternalDecl *Create(Variant& variant, SourceLoc& loc)
      { return new ExternalDecl(variant, loc); }
      
   protected:
   ExternalDecl(Variant& variant, SourceLoc& loc): ASTNode(loc), ASTVariantFeature(variant) {}
   };

   class FunctionDef: public ASTNode {
   public:
      DeclSpecs *specs() const { return specs_; }
      Declarator *declarator() const { return declarator_; }
      CompoundStat *comp_stat() const { return comp_stat_; }

      static FunctionDef *Create(DeclSpecs *specs, Declarator *declarator, CompoundStat *comp_stat,
                                 SourceLoc& loc)
      { return new FunctionDef(specs, declarator, comp_stat, loc); }
      
   protected:
      DeclSpecs *specs_;
      Declarator *declarator_;
      CompoundStat *comp_stat_;

   FunctionDef(DeclSpecs *specs, Declarator *declarator, CompoundStat *comp_stat,
               SourceLoc& loc): ASTNode(loc), specs_(specs), declarator_(declarator),
         comp_stat_(comp_stat) {}
      
   };

   
   class Decl: public ASTNode {
   public:
      DeclSpecs *specs() const { return specs_; }
      
      static Decl *Create(DeclSpecs *specs, SourceLoc& loc) { return new Decl(specs, loc); }
      
   protected:
      DeclSpecs *specs_;
      Declarators *declarators_;

   Decl(DeclSpecs *specs, SourceLoc& loc): ASTNode(loc), specs_(specs) {}
   };
   
   class DeclSpec: public ASTNode, public ASTVariantFeature<TypeSpec> {
   public:
   protected:
   DeclSpec(Variant& variant, SourceLoc& loc): ASTNode(loc), ASTVariantFeature(variant) {}
   };
   
   class TypeSpec: public ASTNode {
   public:
      enum Kind { VOID, CHAR, SHORT, INT, LONG };
      Kind kind() const { return kind_; }
      
      static TypeSpec *Create(enum Kind kind, SourceLoc& loc) { return new TypeSpec(kind, loc); }
      
   protected:
      Kind kind_;
      
   TypeSpec(Kind kind, SourceLoc& loc): ASTNode(loc), kind_(kind) {}
   };

   class Declarator: public ASTNode {
   public:
      Pointer *ptr() const { return ptr_; }
      DirectDeclarator *direct_decl() const { return direct_decl_; }
      
      static Declarator *Create(Pointer *ptr, DirectDeclarator *direct_decl, SourceLoc& loc)
      { return new Declarator(ptr, direct_decl, loc); }
      
   protected:
      Pointer *ptr_;
      DirectDeclarator *direct_decl_;
      
   Declarator(Pointer *ptr, DirectDeclarator *direct_decl, SourceLoc& loc):
      ASTNode(loc), ptr_(ptr), direct_decl_(direct_decl) {}
   };

   class Pointer: public ASTNode {
   public:
      int depth() const { return depth_; }

      static Pointer *Create(int depth, SourceLoc& loc) { return new Pointer(depth, loc); }
      
   protected:
      int depth_ = 0;

   Pointer(int depth, SourceLoc& loc): ASTNode(loc), depth_(depth) {}
   };

   class DirectDeclarator: public ASTNode, public ASTVariantFeature<Identifier *, Declarator *> {
   public:
      static DirectDeclarator *Create(Variant& variant, SourceLoc& loc)
      { return new DirectDeclarator(variant, loc); }
      
   protected:
   DirectDeclarator(Variant& variant, SourceLoc& loc): ASTNode(loc), ASTVariantFeature(variant) {}
   };

   class ParamDecl: public ASTNode {
   public:
      DeclSpecs *specs() const { return specs_; }
      Declarator *declarator() const { return declarator_; }

      static ParamDecl *Create(DeclSpecs *specs, Declarator *declarator, SourceLoc& loc)
      { return new ParamDecl(specs, declarator, loc); }
      
   protected:
      DeclSpecs *specs_;
      Declarator *declarator_;

   ParamDecl(DeclSpecs *specs, Declarator *declarator, SourceLoc& loc):
      ASTNode(loc), specs_(specs), declarator_(declarator) {}
   };

   
}

#endif
