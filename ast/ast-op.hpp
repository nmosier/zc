#ifndef __AST_HPP
#error "include ast.hpp, not ast/ast-op.hpp directly"
#endif

#ifndef __AST_OP_HPP
#define __AST_OP_HPP

namespace zc {

   class ASTOp: public ASTNode {
   public:
      enum Kind: int;
      const Kind& kind() const { return kind_; }
      
   protected:
      Kind kind_;
      
      ASTOp(Kind kind, SourceLoc& loc): ASTNode(loc), kind_(kind) {}      
   };

   
   class UnaryOperator: public ASTOp {
   public:
      enum Kind {DEREFERENCE, POSITIVE, NEGATIVE, BITWISE_NOT, LOGICAL_NOT};

      static UnaryOperator *Create(Kind kind, SourceLoc& loc)
      { return new UnaryOperator(kind, loc); }
      
   protected:

      UnaryOperator(Kind& kind, SourceLoc& loc): ASTOp(kind, loc) {}
   };

   class BinaryOperator: public ASTOp {
   public:
      enum Kind {ASSIGN, EQ, NEQ, LOGICAL_OR, LOGICAL_AND, BITWISE_OR, BITWISE_AND, BITWISE_XOR,
                 LT, LTEQ, GT, GTEQ};

      static BinaryOperator *Create(Kind kind, SourceLoc& loc) { return BinaryOperator(kind, loc); }
      
   protected:
      BinaryOperator(Kind& kind, SourceLoc& loc): ASTOp(kind, loc) {}
   };

}

#endif
