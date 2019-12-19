#ifndef __AST_H
#error "include ast.h, not ast/ast-stat.h directly"
#endif

#ifndef __AST_STAT_H
#define __AST_STAT_H

namespace zc {

   class Stat: public ASTStat,
      public ASTVariantM<ExprStat, CompoundStat, SelectionStat, IterationStat, JumpStat> {
   public:
      static Stat *Create(Variant& variant, SourceLoc& loc) { return new Stat(variant, loc); }
   protected:
   Stat(Variant& variant, SourceLoc& loc): ASTStat(loc), ASTVariantM(variant) {}
   };

   class CompoundStat: public ASTStat {
   public:
      Decls *decls() const { return decls_; }
      Stats *stats() const { return stats_; }
      
      static CompoundStat *Create(Decls *decls, Stats *stats, SourceLoc& loc)
      { return new CompoundStat(decls, stats, loc); }
      
   protected:
      Decls *decls_;
      Stats *stats_;
      
   CompoundStat(Decls *decls, Stats *stats, SourceLoc& loc):
      ASTStat(loc), decls_(decls), stats_(stats) {}
   };
   
   class ExprStat: public ASTStat {
   public:
      Expr *expr() const { return expr_; }
      
      static ExprStat *Create(Expr *expr, SourceLoc& loc) { return new ExprStat(expr, loc); }
      
   protected:
      Expr *expr_;
      
   ExprStat(Expr *expr, SourceLoc& loc): ASTStat(loc), expr_(expr) {}
   };
   
   class SelectionStat: public Stat {
   public:
   protected:
   };
   
   class IterationStat: public Stat {
   public:
   protected:
   };
   
   class JumpStat: public Stat {
   public:
   protected:
   };

}

#endif
