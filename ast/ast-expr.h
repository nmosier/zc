#ifndef __AST_H
#error "include ast.h, not ast/ast-expr.h directly"
#endif

#ifndef __AST_EXPR_H
#define __AST_EXPR_H

namespace zc {
   
   template <class ExprNode>
   class ASTSubexpr: public ASTExpr {
   public:
      ExprNode *expr() const { return expr_; }
   protected:
      ExprNode *expr_;
      
   ASTSubexpr(ExprNode *expr, SourceLoc& loc): ASTExpr(loc), expr_(expr) {}
   };

   class Expr: public ASTSubexpr<AssignmentExpr> {
   public:
      static Expr *Create(AssignmentExpr *expr, SourceLoc& loc) { return new Expr(expr, loc); }
      
   protected:
   Expr(AssignmentExpr *expr, SourceLoc& loc): ASTSubexpr(expr, loc) {}
   };


   class CastExpr: public ASTSubexpr<UnaryExpr> {
   public:
      TypeName *type_name() const { return type_name_; }
      
      static CastExpr *Create(TypeName *type_name, UnaryExpr *expr, SourceLoc& loc)
      { return new CastExpr(type_name, expr, loc); }
      
   protected:
      TypeName *type_name_;

   CastExpr(TypeName *type_name, UnaryExpr *expr, SourceLoc& loc):
      ASTSubexpr(expr, loc), type_name_(type_name) {}
   };

   class UnaryExpr: public ASTExpr, public ASTVariantM<PostfixExpr, UnaryCastExpr> {
   public:
      static UnaryExpr *Create(Variant& variant, SourceLoc& loc)
      { return new UnaryExpr(variant, loc); }
   protected:
   UnaryExpr(Variant& variant, SourceLoc& loc): ASTExpr(loc), ASTVariantM(variant) {}
   };

   class PostfixExpr: public ASTNode {
   public:
   protected:
   };

   class UnaryCastExpr: public ASTSubexpr<CastExpr> {
   public:
      UnaryOperator *op() const { return op_; }

      static UnaryCastExpr *Create(UnaryOperator *op, CastExpr *expr, SourceLoc& loc)
      { return new UnaryCastExpr(op, expr, loc); }
      
   protected:
      UnaryOperator *op_;

   UnaryCastExpr(UnaryOperator *op, CastExpr *expr, SourceLoc& loc):
      ASTSubexpr(expr, loc), op_(op) {}
   };

}

#endif
