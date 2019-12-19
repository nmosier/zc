#ifndef __AST_HPP
#error "include ast.h, not ast-base.h directly"
#endif

#ifndef __AST_BASE_HPP
#define __AST_BASE_HPP

namespace zc {

   class ASTNode {
   public:
      const SourceLoc& loc() const { return loc_; }
      
   protected:
      SourceLoc loc_;
      
      ASTNode(SourceLoc& loc): loc_(loc) {}
   };

   /* class ASTExpr -- base class for all expressions. 
    * NOTE: No analogue nonterminal in grammar. */


   class ASTStat: public ASTNode {
   public:
   protected:
      ASTStat(SourceLoc& loc): ASTNode(loc) {}
   };
   typedef ASTNodeVec<ASTStat> ASTStats;
   
   template <class Node>
   class ASTVecFeature {
   public:
      typedef std::vector<Node *> Vec;
      Vec& vec() const { return vec_; }

   protected:
      Vec vec_;

      ASTVecFeature() {}
   };

   template <class Node>
   class ASTNodeVec: public ASTNode, public ASTVecFeature<Node> {
   public:
      static ASTNodeVec<Node> *Create(SourceLoc& loc) { return new ASTNodeVec<Node>(loc); }
   protected:
      ASTNodeVec(SourceLoc& loc): ASTNode(loc), ASTVecFeature<Node>() {}
   };

   template <class... Types>
   class ASTVariantFeature {
   public:
      typedef std::variant<Types*...> Variant;
      Variant& variant() const { return variant_; }
      
   protected:
      Variant variant_;

      ASTVariantFeature(Variant& variant): variant_(variant) {}
   };

   class ASTExpr: public ASTNode {
   public:
   protected:
      ASTExpr(SourceLoc& loc): ASTNode(loc) {}
   };

   class ASTUnaryExpr: public ASTExpr {
   public:
      ASTExpr *expr() const { return expr_; }
   protected:
      ASTExpr *expr_;
      ASTUnaryExpr(ASTExpr *expr, SourceLoc& loc): ASTExpr(loc), expr_(expr) {}
   };

   class ASTBinaryExpr: public ASTExpr {
   public:
      ASTExpr *lhs() const { return lhs_; }
      ASTExpr *rhs() const { return rhs_; }
   protected:
      ASTExpr *lhs_;
      ASTExpr *rhs_;
      ASTBinaryExpr(ASTExpr *lhs, ASTExpr *rhs, SourceLoc& loc):
         ASTExpr(loc), lhs_(lhs), rhs_(rhs) {}
   };
   

   template <class... Exprs>
   class ASTVariantExpr: public ASTExpr, public ASTVariantFeature<Exprs...> {
      
   };

   template <class Subexpr>
   class ASTVecExpr: public ASTExpr, public ASTVecFeature<Subexpr> {
   public:
      static ASTVecExpr *Create(SourceLoc& loc) { return new ASTVecExpr(loc); }
   protected:
      ASTVecExpr(SourceLoc& loc): ASTExpr(loc), ASTVecFeature<Subexpr>() {}
   };

}

#endif
