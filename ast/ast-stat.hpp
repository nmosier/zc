#ifndef __AST_HPP
#error "include ast.h, not ast/ast-stat.h directly"
#endif

#ifndef __AST_STAT_HPP
#define __AST_STAT_HPP

namespace zc {

   class CompoundStat: public ASTStat {
   public:
      Decls *decls() const { return decls_; }
      ASTStats *stats() const { return stats_; }
      
      static CompoundStat *Create(Decls *decls, ASTStats *stats, SourceLoc& loc) {
         return new CompoundStat(decls, stats, loc);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "CompoundStat"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {
         decls_->Dump(os, level, with_types);
         stats_->Dump(os, level, with_types);
      }

      virtual void TypeCheck(SemantEnv& env, bool scoped = true) override;
      
   protected:
      Decls *decls_;
      ASTStats *stats_;
      
   CompoundStat(Decls *decls, ASTStats *stats, SourceLoc& loc):
      ASTStat(loc), decls_(decls), stats_(stats) {}
   };
   
   class ExprStat: public ASTStat {
   public:
      ASTExpr *expr() const { return expr_; }
      
      static ExprStat *Create(ASTExpr *expr, SourceLoc& loc) { return new ExprStat(expr, loc); }

      virtual void DumpNode(std::ostream& os) const override { os << "ExprStat"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {
         expr_->Dump(os, level, with_types);
      }

      virtual void TypeCheck(SemantEnv& env, bool scoped = true) override;
      
   protected:
      ASTExpr *expr_;
      
   ExprStat(ASTExpr *expr, SourceLoc& loc): ASTStat(loc), expr_(expr) {}
   };
   
}

#endif
