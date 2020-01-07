#ifndef __AST_HPP
#error "include ast.h, not ast/ast-stat.h directly"
#endif

#ifndef __AST_STAT_HPP
#define __AST_STAT_HPP

namespace zc {

   class Block;

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

      /* Semantic Analysis */
      virtual void TypeCheck(SemantEnv& env) override { return TypeCheck(env, true); }
      void TypeCheck(SemantEnv& env, bool scoped);

      /* Code Generation */
      virtual Block *CodeGen(CgenEnv& env, Block *block) override {
         return CodeGen(env, block, false);
      }
      Block *CodeGen(CgenEnv& env, Block *block, bool new_scope);
      virtual void FrameGen(StackFrame& env) const override;
      
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

      /* Semantic Analysis */
      virtual void TypeCheck(SemantEnv& env) override;

      /* Code Generation */
      virtual Block *CodeGen(CgenEnv& env, Block *block) override;
      virtual void FrameGen(StackFrame& env) const override {}
      
   protected:
      ASTExpr *expr_;
      
   ExprStat(ASTExpr *expr, SourceLoc& loc): ASTStat(loc), expr_(expr) {}
   };

   /* NOTE: abstract */
   class JumpStat: public ASTStat {
   public:
   protected:
      template <typename... Args> JumpStat(Args... args): ASTStat(args...) {}
   };

   class ReturnStat: public JumpStat {
   public:
      ASTExpr *expr() const { return expr_; }

      template <typename... Args> static ReturnStat *Create(Args... args) {
         return new ReturnStat(args...);
      }
      
      virtual void DumpNode(std::ostream& os) const override { os << "ReturnStat"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {
         expr_->Dump(os, level, with_types);
      }

      /* Semantic Analysis */
      virtual void TypeCheck(SemantEnv& env) override;

      /* Code Generation */
      virtual Block *CodeGen(CgenEnv& env, Block *block) override;
      virtual void FrameGen(StackFrame& env) const override {}    
      
   protected:
      ASTExpr *expr_;

      template <typename... Args>
      ReturnStat(ASTExpr *expr, Args... args): JumpStat(args...), expr_(expr) {}
   };

   /* NOTE: Abstract */
   class SelectionStat: public ASTStat {
   public:
   protected:
      template <typename... Args> SelectionStat(Args... args): ASTStat(args...) {}
   };

   class IfStat: public SelectionStat {
   public:
      ASTExpr *cond() const { return cond_; }
      ASTStat *if_body() const { return if_body_; }
      ASTStat *else_body() const { return else_body_; }

      template <typename... Args> static IfStat *Create(Args... args) {
         return new IfStat(args...);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "IfStat"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_type) const override {
         cond()->Dump(os, level, with_type);
         if_body()->Dump(os, level, with_type);
         if (else_body() != nullptr) {
            else_body()->Dump(os, level, with_type);
         }
      }

      /* Semantic Analysis */
      virtual void TypeCheck(SemantEnv& env) override;

      /* Code Generation */
      virtual Block *CodeGen(CgenEnv& env, Block *block) override;
      virtual void FrameGen(StackFrame& env) const override;          
      
   protected:
      ASTExpr *cond_;
      ASTStat *if_body_;
      ASTStat *else_body_;

      template <typename... Args>
      IfStat(ASTExpr *cond, ASTStat *if_body, ASTStat *else_body, Args... args):
         SelectionStat(args...), cond_(cond), if_body_(if_body), else_body_(else_body) {}
   };

   /* NOTE: Abstract */
   class IterationStat: public ASTStat {
   public:
      ASTExpr *pred() const { return pred_; }
      ASTStat *body() const { return body_; }
      
   protected:
      ASTExpr *pred_;
      ASTStat *body_;
      
      template <typename... Args>
      IterationStat(ASTExpr *pred, ASTStat *body, Args... args):
         ASTStat(args...), pred_(pred), body_(body) {}
   };

   class WhileStat: public IterationStat {
   public:
      template <typename... Args>
      static WhileStat *Create(Args... args) {
         return new WhileStat(args...);
      }

      virtual void DumpNode(std::ostream& os) const override { os << "WhileStat"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;

      /* Semantic Analysis */
      virtual void TypeCheck(SemantEnv& env) override;

      /* Code Generation */
      virtual Block *CodeGen(CgenEnv& env, Block *block) override;
      virtual void FrameGen(StackFrame& env) const override;          
      
   protected:
      template <typename... Args>
      WhileStat(Args... args): IterationStat(args...) {}
   };
   
}

#endif
