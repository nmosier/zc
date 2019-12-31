#ifndef __AST_HPP
#error "include ast.h, not ast-base.h directly"
#endif

#ifndef __AST_BASE_HPP
#define __AST_BASE_HPP

#include <set>

#include "semant.hpp"

namespace zc {

   std::ostream& indent(std::ostream& os, int level);

   class ASTNode {
   public:
      const SourceLoc& loc() const { return loc_; }
      virtual void Dump(std::ostream& os, int level, bool with_types) const;
      virtual void DumpNode(std::ostream& os) const = 0;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const = 0;
      virtual void DumpType(std::ostream& os) const {}

   protected:
      SourceLoc loc_;
      ASTNode(const SourceLoc& loc): loc_(loc) {}
   };

   class ASTStat: public ASTNode {
   public:
      virtual void TypeCheck(SemantEnv& env) = 0;

   protected:
      ASTStat(const SourceLoc& loc): ASTNode(loc) {}
   };

   
   template <class Node>
   class ASTNodeVec: public ASTNode {
   public:
      typedef std::vector<Node *> Vec;
      Vec& vec() { return vec_; }
      const Vec& vec() const { return vec_; }

      template <typename... Args> static ASTNodeVec<Node> *Create(Args... args) {
         return new ASTNodeVec<Node>(args...);
      }

      virtual void DumpNode(std::ostream& os) const override { os << name(); }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {
         for (const Node *node : vec_) {
            node->Dump(os, level, with_types);
         }
      }

      template <typename... Args>
      void TypeCheck(SemantEnv& env, Args... args) {
         for (Node *node : vec_) {
            node->TypeCheck(env, args...);
         }
      }

      template <typename... Args>
      void CodeGen(CgenEnv& env, Args... args) {
         for (const Node *node : vec()) {
            node->CodeGen(env, args...);
         }
      }

   protected:
      Vec vec_;
      const char *name() const { return "ASTNodeVec"; }
      
      template <typename... Args> ASTNodeVec(Vec vec, Args... args): ASTNode(args...), vec_(vec) {}
      template <typename... Args> ASTNodeVec(Node *node, Args... args):
         ASTNode(args...), vec_(1, node) {}
      template <class OtherNode, typename Func, typename... Args>
      ASTNodeVec(ASTNodeVec<OtherNode> *other, Func func, Args... args): ASTNode(args...) {
         vec_.resize(other->vec().size());
         std::transform(other->vec().begin(), other->vec().end(), vec_.begin(), func);
      }
      template <typename... Args> ASTNodeVec(Args... args): ASTNode(args...) {}
   };

   typedef ASTNodeVec<ASTStat> ASTStats;
   template <> const char *ASTStats::name() const;
   

   template <typename Spec>
   class ASTSpecs: public ASTNode {
      static_assert(std::is_enum<Spec>::value);
   public:
      typedef std::multiset<Spec> Specs;
      const Specs& specs() const { return specs_; }

      template <typename... Args> static ASTSpecs *Create(Args... args) {
         return new ASTSpecs(args...);
      }
      
      virtual void DumpNode(std::ostream& os) const override {
         os << name();
         for (Spec spec : specs_) {
            os << " " << spec;
         }
      }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {}

      /* experimental */
      void insert(Spec spec) { specs_.insert(spec); }

   protected:
      Specs specs_;
      const char *name() const { return "ASTSpecs"; }

      ASTSpecs(const SourceLoc& loc): ASTNode(loc) {}
      
   };
   
   template <class... Types>
   class ASTVariantFeature {
   public:
      typedef std::variant<Types*...> Variant;
      const Variant& variant() const { return variant_; }
      
   protected:
      Variant variant_;

      ASTVariantFeature(const Variant& variant): variant_(variant) {}
   };

   class ASTExpr: public ASTNode {
   public:
      ASTType *type() const { return type_; }

      /*! Enumeration of value kind. */
      enum class ExprKind
         {EXPR_NONE,   /*!< indeterminate; this is the default upon construction */
          EXPR_LVALUE, /*!< expression is an `lvalue'; it can appear on the left-hand side 
                        *   of an assignment */
          EXPR_RVALUE  /*!< expression is an `rvalue'; it can appear on the right-hand side
                        *   of an assignment */
         };
      virtual ExprKind expr_kind() const = 0;

      virtual void TypeCheck(SemantEnv& env) = 0;

      void DumpType(std::ostream& os) const;

   protected:
      /**
       * Type of expression; populated by @see TypeCheck()
       */
      ASTType *type_;
      
      ASTExpr(const SourceLoc& loc): ASTNode(loc), type_(nullptr) {}
   };

   class ASTUnaryExpr: public ASTExpr {
   public:
      ASTExpr *expr() const { return expr_; }

      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;
      
   protected:
      ASTExpr *expr_;
      ASTUnaryExpr(ASTExpr *expr, const SourceLoc& loc): ASTExpr(loc), expr_(expr) {}
   };

   class ASTBinaryExpr: public ASTExpr {
   public:
      ASTExpr *lhs() const { return lhs_; }
      ASTExpr *rhs() const { return rhs_; }
      
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;
      
   protected:
      ASTExpr *lhs_;
      ASTExpr *rhs_;
      ASTBinaryExpr(ASTExpr *lhs, ASTExpr *rhs, const SourceLoc& loc):
         ASTExpr(loc), lhs_(lhs), rhs_(rhs) {}
   };

}

#endif
