#ifndef __AST_HPP
#error "include ast.hpp, not ast/ast-expr.h directly"
#endif

#ifndef __AST_EXPR_HPP
#define __AST_EXPR_HPP

#include <cstdint>
#include <variant>

#include "symtab.hpp"

namespace zc {

   typedef ASTNodeVec<ASTExpr> ASTExprs;

   class AssignmentExpr: public ASTBinaryExpr {
   public:
      virtual ExprKind expr_kind() const override;
      virtual bool is_const() const override { return false; }

      static AssignmentExpr *Create(ASTExpr *lhs, ASTExpr *rhs, const SourceLoc& loc) {
         return new AssignmentExpr(lhs, rhs, loc);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "AssignmentExpr"; }

      virtual void TypeCheck(SemantEnv& env) override;
      virtual Block *CodeGen(CgenEnv& env, Block *block, ExprKind mode) override;

   protected:
      AssignmentExpr(ASTExpr *lhs, ASTExpr *rhs, const SourceLoc& loc):
         ASTBinaryExpr(lhs, rhs, loc) {}
   };

   class UnaryExpr: public ASTUnaryExpr {
   public:
      enum class Kind {UOP_ADDR,
                       UOP_DEREFERENCE,
                       UOP_POSITIVE,
                       UOP_NEGATIVE,
                       UOP_BITWISE_NOT,
                       UOP_LOGICAL_NOT,
                       UOP_INC_PRE,
                       UOP_INC_POST,
                       UOP_DEC_PRE,
                       UOP_DEC_POST
      };
      Kind kind() const { return kind_; }
      const char *kindstr() const;
      virtual ExprKind expr_kind() const override;
      virtual bool is_const() const override;
      virtual intmax_t int_const() const override;
      
      static UnaryExpr *Create(Kind kind, ASTExpr *expr, const SourceLoc& loc) {
         return new UnaryExpr(kind, expr, loc);
      }

      virtual void DumpNode(std::ostream& os) const override;

      virtual void TypeCheck(SemantEnv& env) override;

      virtual Block *CodeGen(CgenEnv& env, Block *block, ExprKind mode) override;

   protected:
      Kind kind_;
      UnaryExpr(Kind kind, ASTExpr *expr, const SourceLoc& loc): ASTUnaryExpr(expr, loc), kind_(kind) {}
   };

   class BinaryExpr: public ASTBinaryExpr {
   public:
      enum class Kind {BOP_LOGICAL_AND,
                       BOP_BITWISE_AND,
                       BOP_LOGICAL_OR,
                       BOP_BITWISE_OR,
                       BOP_BITWISE_XOR,
                       BOP_EQ,
                       BOP_NEQ,
                       BOP_LT,
                       BOP_LEQ,
                       BOP_GT,
                       BOP_GEQ,
                       BOP_PLUS,
                       BOP_MINUS,
                       BOP_TIMES,
                       BOP_DIVIDE,
                       BOP_MOD,
                       BOP_COMMA
      };
      Kind kind() const { return kind_; }
      bool is_logical() const;
      virtual bool is_const() const override;
      virtual intmax_t int_const() const override;      

      template <typename... Args>
      static BinaryExpr *Create(Args... args) { return new BinaryExpr(args...); }

      const char *kindstr() const;
      virtual ExprKind expr_kind() const override;      

      virtual void DumpNode(std::ostream& os) const override;
      virtual void TypeCheck(SemantEnv& env) override;

      virtual Block *CodeGen(CgenEnv& env, Block *block, ExprKind mode) override;
      
   protected:
      Kind kind_;

      template <typename... Args>
      BinaryExpr(Kind kind, Args... args):
         ASTBinaryExpr(args...), kind_(kind) {}
   };

   
   
   class LiteralExpr: public ASTExpr {
   public:
      const intmax_t& val() const { return val_; }
      virtual ExprKind expr_kind() const override;
      virtual bool is_const() const override { return true; }
      virtual intmax_t int_const() const override { return val_; }
      
      static LiteralExpr *Create(const intmax_t& val, const SourceLoc& loc) {
         return new LiteralExpr(val, loc);
      }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {
         /* no children */
      }

      virtual void TypeCheck(SemantEnv& env) override;

      virtual Block *CodeGen(CgenEnv& env, Block *block, ExprKind mode) override;
      
   protected:
      intmax_t val_;
      LiteralExpr(const intmax_t& val, const SourceLoc& loc): ASTExpr(loc), val_(val) {}
   };

   class StringExpr: public ASTExpr {
   public:
      const std::string *str() const { return str_; }
      virtual ExprKind expr_kind() const override;
      virtual bool is_const() const override { return true; }
      
      static StringExpr *Create(const std::string& str, const SourceLoc& loc)
      { return new StringExpr(str, loc); }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {
         /* no children */
      }

      virtual void TypeCheck(SemantEnv& env) override;

      virtual Block *CodeGen(CgenEnv& env, Block *block, ExprKind mode) override;
      
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
      Identifier *id() const { return id_; }
      virtual ExprKind expr_kind() const override;
      virtual bool is_const() const override { return is_const_; }
      virtual intmax_t int_const() const override;
      
      static IdentifierExpr *Create(Identifier *id, const SourceLoc& loc) {
         return new IdentifierExpr(id, loc);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "IdentifierExpr"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;

      virtual void TypeCheck(SemantEnv& env) override;

      virtual Block *CodeGen(CgenEnv& env, Block *block, ExprKind mode) override;
      
   protected:
      Identifier *id_;
      bool is_const_;
      
      IdentifierExpr(Identifier *id, const SourceLoc& loc):
         ASTExpr(loc), id_(id), is_const_(false) {}
   };

   class NoExpr: public ASTExpr {
   public:
      virtual ExprKind expr_kind() const override { return ExprKind::EXPR_NONE; }
      virtual bool is_const() const override { return false; }
      
      static NoExpr *Create(const SourceLoc& loc) { return new NoExpr(loc); }

      virtual void DumpNode(std::ostream& os) const override { os << "NoExpr"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {
         /* no children */
      }

      virtual void TypeCheck(SemantEnv& env) override;

      virtual Block *CodeGen(CgenEnv& env, Block *block, ExprKind mode) override { return block; }

   protected:
      NoExpr(const SourceLoc& loc): ASTExpr(loc) {}
   };

   class CallExpr: public ASTExpr {
   public:
      virtual ExprKind expr_kind() const override { return ExprKind::EXPR_RVALUE; }
      virtual bool is_const() const override { return false; }
      ASTExpr *fn() const { return fn_; }
      ASTExprs *params() const { return params_; }

      template <typename... Args>
      static CallExpr *Create(Args... args) {
         return new CallExpr(args...);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "CallExpr"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;

      virtual void TypeCheck(SemantEnv& env) override;

      virtual Block *CodeGen(CgenEnv& env, Block *block, ExprKind mode) override;

   protected:
      ASTExpr *fn_;
      ASTExprs *params_;

      template <typename... Args>
      CallExpr(ASTExpr *fn, ASTExprs *params, Args... args):
         ASTExpr(args...), fn_(fn), params_(params) {}
      
   };

   class CastExpr: public ASTExpr {
   public:
      virtual ExprKind expr_kind() const override { return ExprKind::EXPR_RVALUE; }
      ASTExpr *expr() const { return expr_; }
      virtual bool is_const() const override { return expr()->is_const(); }
      virtual intmax_t int_const() const override;

      template <typename... Args>
      static CastExpr *Create(Args... args) { return new CastExpr(args...); }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;

      virtual void TypeCheck(SemantEnv& env) override;

      virtual Block *CodeGen(CgenEnv& env, Block *block, ExprKind mode) override;
      
   protected:
      ASTExpr *expr_;

      template <typename... Args>
      CastExpr(ASTType *type, ASTExpr *expr, Args... args):
         ASTExpr(type, args...), expr_(expr) {}
   };

   /**
    * Struct member access expression.
    */
   class MembExpr: public ASTExpr {
   public:
      ASTExpr *expr() const { return expr_; }
      Symbol *memb() const { return memb_; }
      virtual ExprKind expr_kind() const override { return ExprKind::EXPR_LVALUE; }
      virtual bool is_const() const override { return false; }

      template <typename... Args>
      static MembExpr *Create(Args... args) { return new MembExpr(args...); }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;

      virtual void TypeCheck(SemantEnv& env) override;

      virtual Block *CodeGen(CgenEnv& env, Block *block, ExprKind mode) override;      
      
   protected:
      ASTExpr *expr_;
      Symbol *memb_;

      template <typename... Args>
      MembExpr(ASTExpr *expr, Symbol *memb, Args... args):
         ASTExpr(args...), expr_(expr), memb_(memb) {}
   };

   class SizeofExpr: public ASTExpr {
      typedef std::variant<ASTType *, ASTExpr *> Variant;
   public:
      virtual ExprKind expr_kind() const override { return ExprKind::EXPR_RVALUE; }
      virtual bool is_const() const override { return true; }
      virtual intmax_t int_const() const override;
      
      template <typename... Args>
      static SizeofExpr *Create(Args... args) { return new SizeofExpr(args...); }

      virtual void DumpNode(std::ostream& os) const override { os << "DumpNode"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;

      virtual void TypeCheck(SemantEnv& env) override;

      virtual Block *CodeGen(CgenEnv& env, Block *block, ExprKind mode) override;
      
   protected:
      Variant variant_;

      template <class V, typename... Args>
      SizeofExpr(const V& v, Args... args): ASTExpr(args...), variant_(v) {}
   };
   
   class IndexExpr: public ASTExpr {
   public:
      virtual ExprKind expr_kind() const override { return ExprKind::EXPR_LVALUE; }
      ASTExpr *base() const { return base_; }
      ASTExpr *index() const { return index_; }
      virtual bool is_const() const override { return false; } /* TODO: is actually const if
                                                                * expr is string constant.
                                                                * Need to create CONST enum
                                                                * first. */
      // virtual intmax_t int_const() const override;

      template <typename... Args>
      static IndexExpr *Create(Args... args) { return new IndexExpr(args...); }

      virtual void DumpNode(std::ostream& os) const override { os << "IndexExpr"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;
      virtual void TypeCheck(SemantEnv& env) override;
      virtual Block *CodeGen(CgenEnv& env, Block *block, ExprKind mode) override;
      
   protected:
      ASTExpr *base_;
      ASTExpr *index_;
      
      template <typename... Args>
      IndexExpr(ASTExpr *base, ASTExpr *index, Args... args):
         ASTExpr(args...), base_(base), index_(index) {}
   };
   
}

#endif
