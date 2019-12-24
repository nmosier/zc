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
      virtual void DumpNode(std::ostream& os) const {} // = 0;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const {} // = 0;

      /** 
       * Perform semantic analysis on node and all children.
       * @param env the semantic environment
       * @param scoped whether this node is allowed to create a new scope
       */
      virtual void TypeCheck(SemantEnv& env, bool scoped = true) {} // = 0;
      
   protected:
      SourceLoc loc_;
      ASTNode(const SourceLoc& loc): loc_(loc) {}
   };

   class ASTStat: public ASTNode {
   public:
   protected:
      ASTStat(const SourceLoc& loc): ASTNode(loc) {}
   };
   const char ASTStats_s[] = "Stats";
   typedef ASTNodeVec<ASTStat,ASTStats_s> ASTStats;


   
   template <class Node, const char *name>
   class ASTNodeVec: virtual public ASTNode {
   public:
      typedef std::vector<Node *> Vec;
      Vec& vec() { return vec_; }

      static ASTNodeVec<Node,name> *Create(const SourceLoc& loc) { return new ASTNodeVec(loc); }

      virtual void DumpNode(std::ostream& os) const override { os << name; }
      
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {
         for (const Node *node : vec_) {
            node->Dump(os, level, with_types);
         }
      }

      virtual void TypeCheck(SemantEnv& env, bool scoped = true) override {
         for (Node *node : vec_) {
            node->TypeCheck(env, true);
         }
      }

   protected:
      Vec vec_;

      ASTNodeVec(const SourceLoc& loc): ASTNode(loc) {}
   };

   template <typename Spec, const char *name>
   class ASTSpecs: public ASTNode {
      static_assert(std::is_enum<Spec>::value);
   public:
      typedef std::multiset<Spec> Specs;
      const Specs& specs() const { return specs_; }

      static ASTSpecs *Create(const SourceLoc& loc) { return new ASTSpecs(loc); }
      
      virtual void DumpNode(std::ostream& os) const override {
         os << name;
         for (Spec spec : specs_) {
            os << " " << spec;
         }
      }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {}

      /* experimental */
      void insert(Spec spec) { specs_.insert(spec); }

   protected:
      Specs specs_;

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
      Decl *type() const { return type_; }

      /*! Enumeration of value kind. */
      enum class ExprKind
         {EXPR_NONE,   /*!< indeterminate; this is the default upon construction */
          EXPR_LVALUE, /*!< expression is an `lvalue'; it can appear on the left-hand side 
                        *   of an assignment */
          EXPR_RVALUE  /*!< expression is an `rvalue'; it can appear on the right-hand side
                        *   of an assignment */
         };
      virtual ExprKind expr_kind() const = 0;

      virtual void Dump(std::ostream& os, int level, bool with_types) const override;
      void DumpType(std::ostream& os) const;

   protected:
      /**
       * Type of expression; populated by @see TypeCheck()
       */
      Decl *type_;

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
