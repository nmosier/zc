#ifndef __AST_H
#error "include ast.h, not ast-base.h directly"
#endif

#ifndef __AST_BASE_H
#define __AST_BASE_H

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
   class ASTExpr: public ASTNode {
   public:
   protected:
      ASTExpr(SourceLoc& loc): ASTNode(loc) {}
   };

   class ASTOp: public ASTNode {
   public:
   protected:
      ASTOp(SourceLoc& loc): ASTNode(loc) {}      
   };

   class ASTStat: public ASTNode {
   public:
   protected:
      ASTStat(SourceLoc& loc): ASTNode(loc) {}
   };

   
   template <class Node>
   class ASTVecM {
   public:
      typedef std::vector<Node *> Vec;
      Vec& vec() const { return vec_; }

   protected:
      Vec vec_;
   };

   template <class Node>
   class ASTNodeVec: public ASTNode, public ASTVecM<Node> {
   public:
      static ASTNodeVec<Node> *Create(SourceLoc& loc) { return new ASTNodeVec<Node>(loc); }
   protected:
      ASTNodeVec(SourceLoc& loc): ASTNode(loc) {}
   };

   template <class... Types>
   class ASTVariantM {
   public:
      typedef std::variant<Types*...> Variant;
      Variant& variant() const { return variant_; }
      
   protected:
      Variant variant_;

      ASTVariantM(Variant& variant): variant_(variant) {}
   };

   template <class Op>
   class ASTOpM {
      /* Ensure the template class is really an op.*/
      static_assert(std::is_base_of<ASTOp, Op>::value); 
   public:
      Op *op() const { return op; }
      
   protected:
      Op *op_;

      ASTOpM(Op *op): op_(op) {}
   };

}

#endif
