#ifndef __AST_HPP
#error "include ast.hpp, not ast/ast-expr.h directly"
#endif

#ifndef __AST_EXPR_HPP
#define __AST_EXPR_HPP

#include <cstdint>

#include "symtab.hpp"

namespace zc {

   class AssignmentExpr: public ASTBinaryExpr {
   public:
      static AssignmentExpr *Create(ASTExpr *lhs, ASTExpr *rhs, const SourceLoc& loc) {
         return new AssignmentExpr(lhs, rhs, loc);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "AssignmentExpr"; }

      virtual void TypeCheck(SemantEnv& env, bool scoped = true) override;
      virtual void ExprKind(SemantEnv& env) override;

   protected:
      AssignmentExpr(ASTExpr *lhs, ASTExpr *rhs, const SourceLoc& loc):
         ASTBinaryExpr(lhs, rhs, loc) {}
   };

   class UnaryExpr: public ASTUnaryExpr {
   public:
      enum Kind {UOP_ADDR, UOP_DEREFERENCE, UOP_POSITIVE, UOP_NEGATIVE,
                 UOP_BITWISE_NOT, UOP_LOGICAL_NOT, NUOPS};
      Kind kind() const { return kind_; }
      static UnaryExpr *Create(Kind kind, ASTExpr *expr, const SourceLoc& loc) {
         return new UnaryExpr(kind, expr, loc);
      }

      virtual void DumpNode(std::ostream& os) const override;

      virtual void TypeCheck(SemantEnv& env, bool scoped = true) override;

   protected:
      Kind kind_;
      UnaryExpr(Kind kind, ASTExpr *expr, const SourceLoc& loc): ASTUnaryExpr(expr, loc), kind_(kind) {}
   };


   
   class BinaryExpr: public ASTBinaryExpr {
   public:
      enum Kind {BOP_LOGICAL_AND, BOP_BITWISE_AND, BOP_LOGICAL_OR, BOP_BITWISE_OR,
                 BOP_BITWISE_XOR, BOP_EQ, BOP_NEQ, BOP_LT, BOP_LEQ, BOP_GT, BOP_GEQ,
                 BOP_PLUS, BOP_MINUS, BOP_TIMES, BOP_DIVIDE, BOP_MOD};
      Kind kind() const { return kind_; }
      
      static BinaryExpr *Create(Kind kind, ASTExpr *lhs, ASTExpr *rhs, const SourceLoc& loc)
      { return new BinaryExpr(kind, lhs, rhs, loc); }

      const char *kindstr() const;

      virtual void DumpNode(std::ostream& os) const override;
      
   protected:
      Kind kind_;
      BinaryExpr(Kind kind, ASTExpr *lhs, ASTExpr *rhs, const SourceLoc& loc):
         ASTBinaryExpr(lhs, rhs, loc), kind_(kind) {}
   };

   
   
   class LiteralExpr: public ASTExpr {
   public:
      const intmax_t& val() const { return val_; }
      
      static LiteralExpr *Create(const intmax_t& val, const SourceLoc& loc) {
         return new LiteralExpr(val, loc);
      }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level) const override { /* no children */ }
      
   protected:
      intmax_t val_;
      LiteralExpr(const intmax_t& val, const SourceLoc& loc): ASTExpr(loc), val_(val) {}
   };

   class StringExpr: public ASTExpr {
   public:
      const std::string *str() const { return str_; }
      static StringExpr *Create(const std::string& str, const SourceLoc& loc)
      { return new StringExpr(str, loc); }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level) const override { /* no children */ }
      
   protected:
      const std::string *str_;
       StringExpr(const std::string& str, const SourceLoc& loc): ASTExpr(loc), str_(nullptr) {
          /* add to global string table */
           if (g_str_tab.find(str) == g_str_tab.end()) {
               g_str_tab[str] = new std::string(str);
           }
           str_ = g_str_tab[str];
      }
   };

   class IdentifierExpr: public ASTExpr {
   public:
      static IdentifierExpr *Create(Identifier *id, const SourceLoc& loc) {
         return new IdentifierExpr(id, loc);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "IdentifierExpr"; }
      virtual void DumpChildren(std::ostream& os, int level) const override;
      
   protected:
      Identifier *id_;
      IdentifierExpr(Identifier *id, const SourceLoc& loc): ASTExpr(loc), id_(id) {}
   };

   class NoExpr: public ASTExpr {
   public:
      static NoExpr *Create(const SourceLoc& loc) { return new NoExpr(loc); }

      virtual void DumpNode(std::ostream& os) const override { os << "NoExpr"; }
      virtual void DumpChildren(std::ostream& os, int level) const override { /* no children */ }
      
   protected:
      NoExpr(const SourceLoc& loc): ASTExpr(loc) {}
   };
}

#endif
