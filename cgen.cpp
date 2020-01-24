#include <numeric>
#include <string>
#include <unordered_set>
#include <ostream>

#include "ast.hpp"
#include "asm.hpp"
#include "optim.hpp"
#include "cgen.hpp"

namespace zc {

   static Block dead_block;
   
   void Cgen(TranslationUnit *root, std::ostream& os, const char *filename) {
      
      CgenEnv env;
      root->CodeGen(env);

      OptimizeIR(env);
      env.DumpAsm(os);
   }

   FunctionImpl::FunctionImpl(const CgenEnv& env, Block *entry, Block *fin):
      entry_(entry), fin_(fin), frame_bytes_(env.ext_env().frame().bytes()) {}
      
   BlockTransitions::BlockTransitions(const Transitions& vec): vec_(vec) {
      /* get mask of conditions */
      /* TODO: this doesn't work. */
   }
   
   void CgenExtEnv::Enter(Symbol *sym, const VarDeclarations *args) {
      sym_env_.Enter(sym);
      frame_ = StackFrame(args);
   }

   void CgenExtEnv::Exit() {
      sym_env_.Exit();
   }

   LabelValue *CgenExtEnv::LabelGen(const Symbol *id) const {
      std::string name = std::string("__CLABEL_") + *id;
      Label *label = new Label(*id);
      return new LabelValue(label);
   }

   VarSymInfo::VarSymInfo(const ExternalDecl *ext_decl): SymInfo() {
      decl_ = dynamic_cast<const VarDeclaration *>(ext_decl->decl());
      const ASTType *type = decl()->type();
      Label *label = new Label(std::string("_") + *ext_decl->sym());
      LabelValue *label_val = new LabelValue(label);

      if (type->kind() == ASTType::Kind::TYPE_FUNCTION ||
          type->kind() == ASTType::Kind::TYPE_ARRAY)
         {
         /* Functions and arrays _always_ behave as addresses */
            addr_ = val_ = label_val;
      } else {
         addr_ = label_val;
         val_ = new MemoryValue(label_val, type->bytes());
      }
   }

   VarSymInfo::VarSymInfo(const Value *addr, const VarDeclaration *decl): SymInfo(), addr_(addr) {
      if (decl->type()->kind() == ASTType::Kind::TYPE_FUNCTION ||
          decl->type()->kind() == ASTType::Kind::TYPE_ARRAY) {
         val_ = addr;
      } else {
         val_ = new MemoryValue(addr, decl->bytes());
      }
   }
   
   /*** STRING CONSTANTS ***/
   void StringConstants::Insert(const std::string& str) {
      if (strs_.find(str) == strs_.end()) {
         strs_[str] = new_label();
      }
   }

   LabelValue *StringConstants::Ref(const std::string& str) {
      Insert(str);

      return new LabelValue(strs_[str]);
   }

   Label *StringConstants::new_label() {
      return new Label(std::string("__") + std::string("strconst_") + std::to_string(counter_++));
   }

   /*** CODE GENERATION ***/

   void TranslationUnit::CodeGen(CgenEnv& env) {
      env.EnterScope();

      for (ExternalDecl *decl : decls()->vec()) {
         decl->CodeGen(env);
      }
      
      env.ExitScope();
   }

   void ExternalDecl::CodeGen(CgenEnv& env) {
      /* enter global variable declaration into global scope */
      if (sym()) {
         SymInfo *info = new VarSymInfo(this);
         env.symtab().AddToScope(sym(), info);
      }
   }

   void FunctionDef::CodeGen(CgenEnv& env) {
      /* enter function into global scope */
      VarSymInfo *info = new VarSymInfo(this);
      const VarDeclarations *args = Type()->get_callable()->params();
      env.symtab().AddToScope(sym(), info);
      env.ext_env().Enter(sym(), args);

      /* enter parameters into subscope */
      env.EnterScope();      

      /* initialize stack frame */
      FrameGen(env.ext_env().frame());

      /* assign argument stack locations */
      for (const VarDeclaration *arg : *args) {
         VarSymInfo *info = env.ext_env().frame().next_arg(arg);
         env.symtab().AddToScope(arg->sym(), info);
      }
      
      Block *start_block = new Block(dynamic_cast<const LabelValue *>(info->addr())->label());
      Block *ret_block = new Block(start_block->label()->Prepend("__frameunset"));

      /* emit prologue and epilogue */
      emit_frameset(env, start_block);
      emit_frameunset(env, ret_block);
      
      Block *end_block = comp_stat()->CodeGen(env, start_block, false);

      /* TODO: check if unconditional return is already present before doing this? */      
      end_block->transitions().vec().push_back(new ReturnTransition(Cond::ANY)); 


      env.impls().impls().push_back(FunctionImpl(env, start_block, ret_block));

      env.ext_env().Exit();
      env.ExitScope();
   }
   
   Block *CompoundStat::CodeGen(CgenEnv& env, Block *block, bool new_scope) {
      if (new_scope) {
         env.EnterScope();
      }

      for (Declaration *decl : *decls()) {
         decl->Declare(env);
      }

      for (ASTStat *stat : stats()->vec()) {
         block = stat->CodeGen(env, block);
      }
      
      if (new_scope) {
         env.ExitScope();
      }

      return block;
   }
   Block *ReturnStat::CodeGen(CgenEnv& env, Block *block) {
      /* generate returned expression */
      block = expr()->CodeGen(env, block, return_rv(expr()->type()->bytes()),
                              ASTExpr::ExprKind::EXPR_RVALUE);
      
      /* append return transition */
      block->transitions().vec().push_back(new ReturnTransition(Cond::ANY));

      /* create new dummy block (this should be removed as dead code with optimization) */
      Block *dead_block = new Block(new_label());
      block->transitions().vec().push_back(new JumpTransition(dead_block, Cond::ANY));
      return dead_block;
   }

   Block *ExprStat::CodeGen(CgenEnv& env, Block *block) {
      /* pass out = nullptr, since result is thrown away */
      return expr()->CodeGen(env, block, nullptr, ASTExpr::ExprKind::EXPR_RVALUE);
   }

   Block *IfStat::CodeGen(CgenEnv& env, Block *block) {

#if 0
      const VariableValue *cond_var = new VariableValue(cond()->type()->size());
      
      /* Evaluate predicate */
      block = cond()->CodeGen(env, block, cond_var, ASTExpr::ExprKind::EXPR_RVALUE);
      
      /* Test predicate */
      emit_nonzero_test(env, block, cond_var);
#endif
      
      /* Create joining label and block. */
      Label *join_label = new_label("if_join");
      Block *join_block = new Block(join_label);

      /* Create if, else Blocks. */
      Label *if_label = new_label("if_block");
      Block *if_block = new Block(if_label);

      Label *else_label = new_label("else_block");
      Block *else_block = new Block(else_label);

      emit_predicate(env, block, cond(), if_block, else_block);

#if 0
      /* Create block transitions to if, else blocks */
      BlockTransition *if_transition = new JumpTransition(if_block, Cond::NZ);
      BlockTransition *else_transition = new JumpTransition(else_block, Cond::ANY);

      block->transitions().vec().push_back(if_transition);
      block->transitions().vec().push_back(else_transition);
#endif

      
      /* Code generate if, else blocks */
      Block *if_end = if_body()->CodeGen(env, if_block);
      Block *else_end;
      if (else_body()) {
         else_end = else_body()->CodeGen(env, else_block);
      } else {
         else_end = else_block;
      }

      /* Transition if, else end blocks to join block */
      BlockTransition *join_transition = new JumpTransition(join_block, Cond::ANY);
      if_end->transitions().vec().push_back(join_transition);
      else_end->transitions().vec().push_back(join_transition);

      return join_block;
   }

   Block *LoopStat::CodeGen(CgenEnv& env, Block *block) {
      Label *body_label = new_label("loop_body");
      Block *body_block = new Block(body_label);
      BlockTransition *body_trans = new JumpTransition(body_block, Cond::ANY);
      
      Label *join_label = new_label("loop_join");
      Block *join_block = new Block(join_label);

      StatInfo *stat_info = new StatInfo(this, body_block, join_block);
      env.stat_stack().Push(stat_info);
      
      block->transitions().vec().push_back(body_trans);
      auto end_block = body()->CodeGen(env, body_block);
      end_block->transitions().vec().push_back(body_trans);
      
      env.stat_stack().Pop();
      
      return join_block;
   }

   Block *WhileStat::CodeGen(CgenEnv& env, Block *block) {
      Label *body_label = new_label("while_body");
      Block *body_block = new Block(body_label);
      
      Label *pred_label = new_label("while_pred");
      Block *pred_block = new Block(pred_label);

      Label *join_label = new_label("while_end");
      Block *join_block = new Block(join_label);

      /* push stat info onto statement stack */
      StatInfo *stat_info = new StatInfo(this, body_block, join_block);
      env.stat_stack().Push(stat_info);

#if 0
      pred_block = pred()->CodeGen(env, pred_block, pred_var ASTExpr::ExprKind::EXPR_RVALUE);
      emit_nonzero_test(env, pred_block, pred_var);
      pred_block->transitions().vec().push_back(new JumpTransition(body_block, Cond::NZ));
      pred_block->transitions().vec().push_back(new JumpTransition(join_block, Cond::ANY));
#else
      emit_predicate(env, block, pred(), body_block, join_block);
#endif

      body_block = body()->CodeGen(env, body_block);
      body_block->transitions().vec().push_back(new JumpTransition(pred_block, Cond::ANY));

      block->transitions().vec().push_back(new JumpTransition(pred_block, Cond::ANY));

      env.stat_stack().Pop();

      return join_block;
   }

   Block *ForStat::CodeGen(CgenEnv& env, Block *block) {
      Label *cond_label = new_label("for_cond");
      Block *cond_block = new Block(cond_label);
      BlockTransition *cond_trans = new JumpTransition(cond_block, Cond::ANY);

      Label *after_label = new_label("for_after");
      Block *after_block = new Block(after_label);
      BlockTransition *after_trans = new JumpTransition(after_block, Cond::ANY);
      
      Label *body_label = new_label("for_body");
      Block *body_block = new Block(body_label);
      // BlockTransition *body_trans = new JumpTransition(body_block, Cond::NZ);

      Label *join_label = new_label("for_join");
      Block *join_block = new Block(join_label);
      // BlockTransition *join_trans = new JumpTransition(join_block, Cond::ANY);

      StatInfo *stat_info = new StatInfo(this, after_block, join_block);
      env.stat_stack().Push(stat_info);
      
      block = init()->CodeGen(env, block, nullptr, ASTExpr::ExprKind::EXPR_RVALUE);
      block->transitions().vec().push_back(cond_trans);

      emit_predicate(env, cond_block, pred(), body_block, join_block);
      
      block = body()->CodeGen(env, body_block);
      block->transitions().vec().push_back(after_trans);
      block = after()->CodeGen(env, after_block, nullptr, ASTExpr::ExprKind::EXPR_RVALUE);
      block->transitions().vec().push_back(cond_trans);
      
      env.stat_stack().Pop();
      
      return join_block;
   }

   Block *BreakStat::CodeGen(CgenEnv& env, Block *block) {
      StatInfo *stat_info = env.stat_stack().Find([](auto info){return info->stat()->can_break(); });
      block->transitions().vec().push_back(new JumpTransition(stat_info->exit(), Cond::ANY));
      return &dead_block;
   }

   Block *ContinueStat::CodeGen(CgenEnv& env, Block *block) {
      StatInfo *stat_info = env.stat_stack().Find([](auto info){return info->stat()->can_break(); });
      block->transitions().vec().push_back(new JumpTransition(stat_info->enter(), Cond::ANY));
      return &dead_block;
   }
   
   Block *GotoStat::CodeGen(CgenEnv& env, Block *block) {
      /* obtain label */
      const LabelValue *lv = env.ext_env().LabelGen(label_id()->id());
      block->instrs().push_back(new JumpInstruction(lv));
      return block;
   }

   Block *LabeledStat::CodeGen(CgenEnv& env, Block *block) {
      return stat()->CodeGen(env, block);
   }
   
   Block *LabelDefStat::CodeGen(CgenEnv& env, Block *block) {
      /* obtain label */
      const LabelValue *lv = env.ext_env().LabelGen(label_id()->id());
      
      /* create new block */
      Block *labeled_block = new Block(lv->label());

      /* link current block to labeled block */
      block->transitions().vec().push_back(new JumpTransition(labeled_block, Cond::ANY));
      block = labeled_block;
      return LabeledStat::CodeGen(env, block);
   }

   Block *AssignmentExpr::CodeGen(CgenEnv& env, Block *block, const Value *out,
                                  ExprKind mode) {
      assert(mode == ExprKind::EXPR_RVALUE);
      
      /* compute right-hand rvalue */
      const VariableValue *rhs_var = new VariableValue(rhs()->type()->bytes());
      block = rhs()->CodeGen(env, block, rhs_var, ExprKind::EXPR_RVALUE);

      /* compute left-hand lvalue */
      const VariableValue *lhs_var = new VariableValue(long_size);
      block = lhs()->CodeGen(env, block, lhs_var, ExprKind::EXPR_LVALUE);

      /* assign */
      const MemoryValue *memval = new MemoryValue(lhs_var, type()->bytes());
      block->instrs().push_back(new LoadInstruction(memval, rhs_var));

      /* propogate result */
      if (out) {
         block->instrs().push_back(new LoadInstruction(out, rhs_var));
      }

      return block;
   }

   Block *UnaryExpr::CodeGen(CgenEnv& env, Block *block, const Value *out, ExprKind mode) {
      int bytes = expr()->type()->bytes();
      const VariableValue *var;
      switch (kind()) {
      case Kind::UOP_ADDR:
         assert(mode == ExprKind::EXPR_RVALUE);
         /* get subexpression as lvalue */
         block = expr()->CodeGen(env, block, out, ExprKind::EXPR_LVALUE);
         break;
         
      case Kind::UOP_DEREFERENCE:
         /* get address as rvalue */
         var = new VariableValue(bytes);
         block = expr()->CodeGen(env, block, var, ExprKind::EXPR_RVALUE);

         /* dereference address into _out_ */
         block->instrs().push_back(new LoadInstruction(out, new MemoryValue(var, type()->bytes())));
         break;
         
      case Kind::UOP_POSITIVE:
         block = expr()->CodeGen(env, block, out, ExprKind::EXPR_RVALUE);         
         break;
         
      case Kind::UOP_NEGATIVE:
         {
            /* select register to load into */
            const RegisterValue *reg;
            switch (bytes) {
            case byte_size: reg = &rv_a; break;
            case word_size: abort();
            case long_size: reg = crt_arg1(bytes); break;
            default:        abort();
            }

            /* compute subexpr into selected register */
            block = expr()->CodeGen(env, block, reg, ExprKind::EXPR_RVALUE);

            /* negate result */
            switch (bytes) {
            case byte_size:
               /* neg */
               block->instrs().push_back(new NegInstruction());
               break;
            case word_size: abort();
            case long_size:
               /* call __ineg */
               emit_crt("__ineg", block);
               break;
            default: abort();
            }

            /* load result into destination */
            block->instrs().push_back(new LoadInstruction(out, reg));
         }
         break;
         
      case Kind::UOP_BITWISE_NOT:
         {
            /* select register to load into */
            const RegisterValue *reg;
            switch (bytes) {
            case byte_size: reg = &rv_a; break;
            case word_size: abort();
            case long_size: reg = crt_arg1(bytes); break;
            default:        abort();
            }

            /* compute subexpr into selected register */
            block = expr()->CodeGen(env, block, reg, ExprKind::EXPR_RVALUE);

            /* bitwise not */
            switch (bytes) {
            case byte_size:
               /* cpl */
               block->instrs().push_back(new CplInstruction());
               break;
            case word_size: abort();
            case long_size:
               /* call __inot */
               emit_crt("__inot", block);
               break;
            default: abort();
            }

            /* load result into destination */
            block->instrs().push_back(new LoadInstruction(out, reg));
         }
         break;
         
      case Kind::UOP_LOGICAL_NOT:
         var = new VariableValue(bytes);
         block = expr()->CodeGen(env, block, var, ExprKind::EXPR_RVALUE);
         emit_logical_not(env, block, var, out);
         break;
         
      case Kind::UOP_INC_PRE:
      case Kind::UOP_DEC_PRE:
      case Kind::UOP_INC_POST:
      case Kind::UOP_DEC_POST:
         block = emit_incdec(env, block,
                             kind() == Kind::UOP_INC_PRE || kind() == Kind::UOP_INC_POST,
                             kind() == Kind::UOP_INC_PRE || kind() == Kind::UOP_DEC_PRE, expr(),
                             out);
         break;
      }

      return block;
   }

   Block *BinaryExpr::CodeGen(CgenEnv& env, Block *block, const Value *out, ExprKind mode) {
      const VariableValue *lhs_var = new VariableValue(lhs()->type()->bytes());
      const VariableValue *rhs_var = new VariableValue(rhs()->type()->bytes());
      
      switch (kind()) {
      case Kind::BOP_LOGICAL_AND:
         /* Short-circuit evaluation dictates that evaluation stops if the first operand 
          * is 0. 
          */
         {
            /* evaluate lhs */
            block = lhs()->CodeGen(env, block, lhs_var, ExprKind::EXPR_RVALUE);
            
            emit_nonzero_test(env, block, lhs_var);

            /* preemptively set result to 0 */
            block->instrs().push_back(new LoadInstruction(out, &imm_b<0>));

            /* create transitions */
            const Label *end_label = new_label("BOP_LOGICAL_AND");
            Block *end_block = new Block(end_label);
            BlockTransition *end_transition = new JumpTransition(end_block, Cond::Z);
            block->transitions().vec().push_back(end_transition);

            const Label *cont_label = new_label("BOP_LOGICAL_AND");
            Block *cont_block = new Block(cont_label);
            BlockTransition *cont_transition = new JumpTransition(end_block, Cond::NZ);
            block->transitions().vec().push_back(cont_transition);

            /* Evaluate rhs */
            cont_block = rhs()->CodeGen(env, cont_block, rhs_var, ExprKind::EXPR_RVALUE);
            emit_booleanize(env, block, rhs_var, out); /* OPTIMIZABLE */
            cont_block->transitions().vec().push_back(end_transition);

            return end_block;
         }
         
      case Kind::BOP_LOGICAL_OR:
         /* Short-circuit evaluation dictates that evaluation continues until an operand is nonzero.
          */
         {
            /* evaluate lhs */
            block = lhs()->CodeGen(env, block, lhs_var, ExprKind::EXPR_RVALUE);
            emit_nonzero_test(env, block, lhs_var);
            
            /* preemptively set out to true */            
            block->instrs().push_back(new LoadInstruction(out, &imm_b<1>)); 
            
            /* create transitions */
            const Label *end_label = new_label("BOP_LOGICAL_OR");
            Block *end_block = new Block(end_label);
            BlockTransition *end_transition = new JumpTransition(end_block, Cond::NZ);
            block->transitions().vec().push_back(end_transition);
            
            const Label *cont_label = new_label("BOP_LOGICAL_OR");
            Block *cont_block = new Block(cont_label);
            BlockTransition *cont_transition = new JumpTransition(end_block, Cond::Z);
            block->transitions().vec().push_back(cont_transition);

            /* Evaluate rhs */
            cont_block = rhs()->CodeGen(env, cont_block, rhs_var, ExprKind::EXPR_RVALUE);
            emit_booleanize(env, block, rhs_var, out);
            cont_block->transitions().vec().push_back(end_transition);
            
            return end_block;
         }

      case Kind::BOP_EQ:
      case Kind::BOP_NEQ:
         {
            bool eq = (kind() == Kind::BOP_EQ);
            
            block = emit_binop(env, block, this, lhs_var, rhs_var);
            switch (lhs_var->size()) {
            case byte_size:
               /* ld a,<lhs>
                * cp a,<rhs>
                * ld <out>,0/1
                * jr nz,_
                * inc/dec <out>
                * _
                */
               block->instrs().push_back(new LoadInstruction(&rv_a, lhs_var));
               block->instrs().push_back(new CompInstruction(&rv_a, rhs_var));
               break;

            case word_size: abort();
            case long_size:
               /* ld hl,<lhs>
                * or a,a
                * sbc hl,<rhs>
                * add hl,<rhs>
                * ld <out>,0
                * jr nz/z,_
                * inc <out>
                * _
                */
               block->instrs().push_back(new LoadInstruction(&rv_hl, lhs_var));
               block->instrs().push_back(new OrInstruction(&rv_a, &rv_a));
               block->instrs().push_back(new SbcInstruction(&rv_hl, rhs_var));
               block->instrs().push_back(new AddInstruction(&rv_hl, rhs_var));
               break;
               
            default: abort();
            }

            block->instrs().push_back(new LoadInstruction(&rv_a, &imm_b<0>));

            Block *join_block = new Block(new_label("BOP_EQ_NEQ"));
            Block *cont_block = new Block(new_label("BOP_EQ_NEQ"));

            block->transitions().vec().push_back
               (new JumpTransition(join_block, eq ? Cond::NZ : Cond::Z));
            block->transitions().vec().push_back(new JumpTransition(cont_block, Cond::ANY));

            cont_block->instrs().push_back(new IncInstruction(out));
            cont_block->transitions().vec().push_back(new JumpTransition(join_block, Cond::ANY));
               
            return join_block;
         }

      case Kind::BOP_LT:
      case Kind::BOP_GT:
         {
            bool lt_not_gt = (kind() == Kind::BOP_LT);
            block = emit_binop(env, block, this, lhs_var, rhs_var);
            switch (lhs()->type()->bytes()) {
            case byte_size:
               /* ld a,<lhs>/<rhs>
                * cp a,<rhs>/<lhs>
                * ld a,0
                */
               block->instrs().push_back(new LoadInstruction(&rv_a, lt_not_gt ? lhs_var : rhs_var));
               block->instrs().push_back(new CompInstruction(&rv_a, lt_not_gt ? rhs_var : lhs_var));
               block->instrs().push_back(new LoadInstruction(&rv_a, &imm_b<0>));
               block->instrs().push_back(new AdcInstruction(&rv_a, &rv_a));
               block->instrs().push_back(new LoadInstruction(out, &rv_a));
               break;
                                  
            case word_size: abort();
            case long_size:
               /* ld hl,<lhs>
                * xor a,a
                * sbc hl,<rhs>
                */
               block->instrs().push_back(new LoadInstruction(&rv_hl, lt_not_gt ? lhs_var : rhs_var));
               block->instrs().push_back(new XorInstruction(&rv_a, &rv_a));
               block->instrs().push_back(new SbcInstruction(&rv_hl, lt_not_gt ? rhs_var : lhs_var));
               block->instrs().push_back(new AdcInstruction(&rv_a, &rv_a));
               break;
            
            default: abort();
            }
         
            block->instrs().push_back(new AdcInstruction(&rv_a, &rv_a));
            block->instrs().push_back(new LoadInstruction(out, &rv_a));
         }
         break;

         
      case Kind::BOP_LEQ:
      case Kind::BOP_GEQ:
         {
            bool leq_not_geq = (kind() == Kind::BOP_LEQ);
            block = emit_binop(env, block, this, lhs_var, rhs_var);
            switch (lhs()->type()->bytes()) {
            case byte_size:
               /* scf
                * ld a,<lhs>/<rhs>
                * sbc a,<rhs>/<lhs>
                * ld a,0
                */
               block->instrs().push_back(new ScfInstruction());
               block->instrs().push_back(new LoadInstruction(&rv_a, lhs_var));
               block->instrs().push_back(new SbcInstruction(&rv_a, rhs_var));
               block->instrs().push_back(new LoadInstruction(&rv_a, &imm_b<0>));
               break;

            case word_size: abort();
            case long_size:
               /* xor a,a
                * scf
                * ld hl,<lhs>/<rhs>
                * sbc hl,<rhs>/<lhs>
                */
               block->instrs().push_back(new XorInstruction(&rv_a, &rv_a));
               block->instrs().push_back(new ScfInstruction());
               block->instrs().push_back(new LoadInstruction(&rv_hl, leq_not_geq ? lhs_var : rhs_var));
               block->instrs().push_back(new SbcInstruction(&rv_hl, leq_not_geq ? rhs_var : lhs_var));
               break;

            default: abort();
            }

            block->instrs().push_back(new AdcInstruction(&rv_a, &rv_a));
         }
         break;

      case Kind::BOP_PLUS:
         block = emit_binop(env, block, this, lhs_var, rhs_var);
         block->instrs().push_back(new LoadInstruction(accumulator(lhs_var), lhs_var));
         block->instrs().push_back(new AddInstruction(accumulator(lhs_var), rhs_var));
         block->instrs().push_back(new LoadInstruction(out, accumulator(lhs_var)));
         break;

      case Kind::BOP_MINUS:
         block = emit_binop(env, block, this, lhs_var, rhs_var);
         switch (lhs_var->size()) {
         case byte_size:
            /* ld a,<lhs>
             * sub a,<rhs>
             * ld <out>,a
             */
            block->instrs().push_back(new LoadInstruction(&rv_a, lhs_var));
            block->instrs().push_back(new SubInstruction(&rv_a, rhs_var));
            block->instrs().push_back(new LoadInstruction(out, &rv_a));
            break;
            
         case word_size: abort();
         case long_size:
            /* or a,a
             * ld hl,<lhs>
             * sbc hl,<rhs>
             * ld <out>,hl
             */
            block->instrs().push_back(new OrInstruction(&rv_a, &rv_a));
            block->instrs().push_back(new LoadInstruction(&rv_hl, lhs_var));
            block->instrs().push_back(new SbcInstruction(&rv_hl, rhs_var));
            block->instrs().push_back(new LoadInstruction(out, &rv_hl));
            break;
         }
         break;

      case Kind::BOP_TIMES:
         block = emit_binop(env, block, this, lhs_var, rhs_var);
         switch (lhs_var->size()) {
         case byte_size:
            /* ld b,<lhs>
             * ld c,<rhs>
             * mlt bc
             * ld <out>,c
             */
            block->instrs().push_back(new LoadInstruction(&rv_b, lhs_var));
            block->instrs().push_back(new LoadInstruction(&rv_c, rhs_var));
            block->instrs().push_back(new MultInstruction(&rv_bc));
            block->instrs().push_back(new LoadInstruction(out, &rv_c));
            break;
            
         case word_size: abort();
         case long_size:
            /* ld hl,<lhs>
             * ld bc,<rhs>
             * call __imulu
             * ld <out>,hl
             */
            block->instrs().push_back(new LoadInstruction(&rv_hl, lhs_var));
            block->instrs().push_back(new LoadInstruction(&rv_bc, rhs_var));
            emit_crt("__imulu", block);
            block->instrs().push_back(new LoadInstruction(out, &rv_hl));
            break;
         }
         break;

      case Kind::BOP_DIVIDE:
      case Kind::BOP_MOD:
         {
            bool div_not_mod = (kind() == Kind::BOP_DIVIDE);
            block = emit_binop(env, block, this, lhs_var, rhs_var);
            block->instrs().push_back(new LoadInstruction(crt_arg1(lhs_var), lhs_var));
            block->instrs().push_back(new LoadInstruction(crt_arg2(rhs_var), rhs_var));
            emit_crt(crt_prefix(lhs_var) + "divu", block);
            if (div_not_mod) {
               block->instrs().push_back(new LoadInstruction(out, crt_arg1(lhs_var)));
            } else {
               block->instrs().push_back(new LoadInstruction(out, crt_arg2(rhs_var)));
            }
         }
         break;
         
      case Kind::BOP_BITWISE_AND:
      case Kind::BOP_BITWISE_OR:
      case Kind::BOP_BITWISE_XOR:
         block = emit_binop(env, block, this, lhs_var, rhs_var);
         switch (lhs_var->size()) {
         case byte_size:
            /* ld a,<lhs>
             * and/or/xor a,<rhs>
             * ld <out>,a
             */
            block->instrs().push_back(new LoadInstruction(&rv_a, lhs_var));
            switch (kind()) {
            case Kind::BOP_BITWISE_AND:
               block->instrs().push_back(new AndInstruction(&rv_a, rhs_var)); break;
            case Kind::BOP_BITWISE_OR:
               block->instrs().push_back(new OrInstruction(&rv_a, rhs_var));  break;
            case Kind::BOP_BITWISE_XOR:
               block->instrs().push_back(new XorInstruction(&rv_a, rhs_var)); break;
            default: abort();
            }
            block->instrs().push_back(new LoadInstruction(out, &rv_a));
            break;
            
         case word_size: abort();
         case long_size:
            /* ld hl,<lhs>
             * ld bc,<rhs>
             * call __iand/__ior/__ixor
             * ld <out>,hl
             */
            block->instrs().push_back(new LoadInstruction(&rv_hl, lhs_var));
            block->instrs().push_back(new LoadInstruction(&rv_bc, rhs_var));
            switch (kind()) {
            case Kind::BOP_BITWISE_AND: emit_crt("__iand", block); break;
            case Kind::BOP_BITWISE_OR:  emit_crt("__ior", block);  break;
            case Kind::BOP_BITWISE_XOR: emit_crt("__ixor", block); break;
            default: abort();
            }
            block->instrs().push_back(new LoadInstruction(out, &rv_hl));
            break;
         }
         break;

      case Kind::BOP_COMMA:
         lhs()->CodeGen(env, block, nullptr, ExprKind::EXPR_RVALUE);
         rhs()->CodeGen(env, block, out, mode);
         break;
      }

      return block;
   }
   
   Block *LiteralExpr::CodeGen(CgenEnv& env, Block *block, const Value *out, ExprKind mode) {
      const ImmediateValue *imm = new ImmediateValue(val(), type()->bytes());
      block->instrs().push_back(new LoadInstruction(out, imm));
      return block;
   }
   
   Block *StringExpr::CodeGen(CgenEnv& env, Block *block, const Value *out, ExprKind mode) {
      assert(mode == ExprKind::EXPR_RVALUE); /* string constants aren't assignable */
      block->instrs().push_back(new LoadInstruction(out, env.strconsts().Ref(*str())));
      return block;
   }
   
   Block *IdentifierExpr::CodeGen(CgenEnv& env, Block *block, const Value *out, ExprKind mode) {
      const SymInfo *id_info = env.symtab().Lookup(id()->id());
      
      if (mode == ExprKind::EXPR_LVALUE || type()->kind() == ASTType::Kind::TYPE_ARRAY) {
         /* obtain address of identifier */
         const Value *id_addr = dynamic_cast<const VarSymInfo *>(id_info)->addr();
         block->instrs().push_back(new LeaInstruction(out, id_addr));
      } else {
         const Value *id_rval = id_info->val();
         const RegisterValue *rv;
         block->instrs().push_back(new LoadInstruction(out, id_rval));
      }

      return block;
   }

   Block *CallExpr::CodeGen(CgenEnv& env, Block *block, const Value *out, ExprKind mode) {
      /* push arguments onto stack (1st arg is highest on stack) */

      /* codegen params */
      for (auto it = params()->vec().rbegin(), end = params()->vec().rend(); it != end; ++it) {
         auto param = *it;
         int param_bytes = param->type()->Decay()->bytes();
         const VariableValue *param_var = new VariableValue(param_bytes);
         block = param->CodeGen(env, block, param_var, ExprKind::EXPR_RVALUE);
         block->instrs().push_back(new PushInstruction(param_var));
      }

      /* codegen callee */
      block = fn()->CodeGen(env, block, &rv_iy, ExprKind::EXPR_RVALUE);

      emit_crt("__indcall", block);
      
      /* pop off args */
      for (ASTExpr *param : params()->vec()) {
         /* TODO: this could be optimized. */
         block->instrs().push_back(new PopInstruction(&rv_de)); /* pop off into scrap register */
      }
      
      return block;
   }

   Block *CastExpr::CodeGen(CgenEnv& env, Block *block, const Value *out, ExprKind mode) {
      assert(mode == ExprKind::EXPR_RVALUE);

      const VariableValue *var = new VariableValue(expr()->type()->bytes());
      block = expr()->CodeGen(env, block, var, mode);
      block->instrs().push_back(new LoadInstruction(accumulator(var), var));
      accumulator(type()->bytes())->reg()->Cast(block, &r_hl);
      block->instrs().push_back(new LoadInstruction(out, accumulator(var)));

      return block;
   }

   Block *MembExpr::CodeGen(CgenEnv& env, Block *block, const Value *out, ExprKind mode) {
      /* code generate struct as lvalue */
      block = expr()->CodeGen(env, block, &rv_hl, ExprKind::EXPR_LVALUE);
      
      /* get internal offset */
      int offset = dynamic_cast<CompoundType *>(expr()->type())->offset(memb());
      ImmediateValue *imm = new ImmediateValue(offset, long_size);

      const VariableValue *offset_var = new VariableValue(long_size);
      /* ld <off>,#off
       * add hl,<off>
       */
      block->instrs().push_back(new LoadInstruction(offset_var, imm));
      block->instrs().push_back(new AddInstruction(&rv_hl, offset_var));
      
      switch (mode) {
      case ExprKind::EXPR_LVALUE:
         block->instrs().push_back(new LoadInstruction(out, &rv_hl));
         break; /* already computed address */
         
      case ExprKind::EXPR_RVALUE:
         /* ld <out>,(hl) */
         block->instrs().push_back
            (new LoadInstruction(out, new MemoryValue(&rv_hl, type()->bytes())));
         break;

      case ExprKind::EXPR_NONE: abort();
      }

      return block;
   }

   Block *SizeofExpr::CodeGen(CgenEnv& env, Block *block, const Value *out, ExprKind mode) {
      struct CodeGenVisitor {
         int operator()(const ASTType *type) { return type->bytes(); }
         int operator()(const ASTExpr *expr) { return expr->type()->bytes(); }
      };

      int imm = std::visit(CodeGenVisitor(), variant_);
      if (out) {
         block->instrs().push_back(new LoadInstruction(out, new ImmediateValue(imm, out->size())));
      }

      return block;
   }

   Block *IndexExpr::CodeGen(CgenEnv& env, Block *block, const Value *out, ExprKind mode) {
      /* generate index */
      block = index()->CodeGen(env, block, &rv_hl, ExprKind::EXPR_RVALUE);
      block->instrs().push_back
         (new LoadInstruction(&rv_bc, new ImmediateValue(type()->bytes(), long_size)));
      emit_crt("__imuls", block);
      const VariableValue *index_var = new VariableValue(long_size);
      block->instrs().push_back(new LoadInstruction(index_var, &rv_hl));

      /* generate base */
      block = base()->CodeGen(env, block, &rv_hl, ExprKind::EXPR_LVALUE);
      block->instrs().push_back(new AddInstruction(&rv_hl, index_var));

      switch (mode) {
      case ExprKind::EXPR_LVALUE:
         block->instrs().push_back(new LoadInstruction(out, &rv_hl));
         break;
         
      case ExprKind::EXPR_RVALUE:
         block->instrs().push_back
            (new LoadInstruction(out, new MemoryValue(&rv_hl, type()->bytes())));
         break;
         
      case ExprKind::EXPR_NONE: abort();
      }
      return block;
   }


   
   /*** OTHER ***/

   static int label_counter = 0;

   const Register *return_register(const Value *val) { return return_register(val->size()); }
   const Register *return_register(int bytes) {
      switch (bytes) {
      case byte_size: return &r_a;
      case word_size: return &r_hl;
      case long_size: return &r_hl;
      default:        abort();
      }
   }

   const RegisterValue *return_rv(const Value *val) { return return_rv(val->size()); }
   const RegisterValue *return_rv(int bytes) {
      switch (bytes) {
      case byte_size: return &rv_a;
      case word_size: return &rv_hl;
      case long_size: return &rv_hl;
      default:        abort();
      }
   }

   const RegisterValue *crt_arg1(const Value *val) { return crt_arg1(val->size()); }
   const RegisterValue *crt_arg1(int bytes) {
      switch (bytes) {
      case byte_size: return &rv_a;
      case word_size: return &rv_hl;
      case long_size: return &rv_hl;
      default:        abort();
      }
   }

   const RegisterValue *crt_arg2(const Value *val) { return crt_arg2(val->size()); }   
   const RegisterValue *crt_arg2(int bytes) {
      switch (bytes) {
      case byte_size: return &rv_b;
      case word_size: return &rv_bc;
      case long_size: return &rv_bc;
      default:        abort();
      }
   }

   std::string crt_prefix(const Value *val) { return crt_prefix(val->size()); }
   std::string crt_prefix(int bytes) {
      switch (bytes) {
      case byte_size: return "__b";
      case word_size: return "__s";
      case long_size: return "__i";
      default:        abort();
      }
   }

   const RegisterValue *accumulator(const Value *val) { return accumulator(val->size()); }
   const RegisterValue *accumulator(int bytes) {
      switch (bytes) {
      case byte_size: return &rv_a;
      case word_size: abort();
      case long_size: return &rv_hl;
      default:        abort();
      }
   }

   Label *new_label() { return new_label(std::string()); }
   Label *new_label(const std::string& prefix) {
      std::string s = std::string("__LABEL_") + prefix + "_" + std::to_string(label_counter++);
      return new Label(s);
   }

   void emit_nonzero_test(CgenEnv& env, Block *block, const Value *in) {
      int bytes = in->size();
      switch (bytes) {
      case byte_size:
         /* ld a,<out>
          * or a,a
          */
         block->instrs().push_back(new LoadInstruction(new RegisterValue(&r_a), in));
         block->instrs().push_back
            (new OrInstruction(new RegisterValue(&r_a), new RegisterValue(&r_a)));
         break;
         
      case word_size: abort();
      case long_size:
         /* ld hl,<in> 
          * call _icmpzero
          */
         block->instrs().push_back(new LoadInstruction(&rv_hl, in));
         emit_crt("__icmpzero", block);
         break;
      }
   }

   /* NOTE: Doesn't return block because it's sealed by transitions.
    */
   void emit_predicate(CgenEnv& env, Block *block, ASTExpr *expr, Block *take, Block *leave) {
      const VariableValue *var = new VariableValue(expr->type()->bytes());
      block = expr->CodeGen(env, block, var, ASTExpr::ExprKind::EXPR_RVALUE);
      emit_nonzero_test(env, block, var);

      /* transitions */
      block->transitions().vec().push_back(new JumpTransition(take, Cond::NZ));
      block->transitions().vec().push_back(new JumpTransition(leave, Cond::Z));
   }

   void emit_logical_not(CgenEnv& env, Block *block, const Value *in, const Value *out) {
      Instructions& is = block->instrs();

      int bytes = in->size();
      switch (bytes) {
      case byte_size:
         /* ld a,<in>
          * cp a,1
          * ld a,0
          * adc a,a
          * ld <out>,a
          */
         is.push_back(new LoadInstruction(&rv_a, in));
         is.push_back(new CompInstruction(&rv_a, &imm_b<0>));
         is.push_back(new LoadInstruction(&rv_a, &imm_b<0>));
         is.push_back(new AdcInstruction(&rv_a, &rv_a));
         is.push_back(new LoadInstruction(out, &rv_a));
         break;

      case word_size: abort();
         
      case long_size:
         /* or a,a
          * sbc hl,hl  ; ld hl,0
          * sbc hl,<in> ; CF <- <in> is nonzero
          * sbc hl,hl ; ld hl,-1 (c), ld hl,0 (nc)
          * inc hl
          * ld <out>,hl
          */
         is.push_back(new OrInstruction(&rv_a, &rv_a));
         is.push_back(new SbcInstruction(&rv_hl, &rv_hl));
         is.push_back(new SbcInstruction(&rv_hl, in));
         is.push_back(new SbcInstruction(&rv_hl, &rv_hl));
         is.push_back(new IncInstruction(&rv_hl));
         is.push_back(new LoadInstruction(out, &rv_hl));
         break;
      }
   }

   void emit_booleanize(CgenEnv& env, Block *block, const Value *in, const Value *out) {
      Instructions& is = block->instrs();

      int bytes = in->size();
      switch (bytes) {
      case byte_size:
         /* xor a,a
          * cp a,<in>
          * adc a,a
          * ld <out>,a
          */
         is.push_back(new XorInstruction(&rv_a, &rv_a));
         is.push_back(new CompInstruction(&rv_a, in));
         is.push_back(new AdcInstruction(&rv_a, &rv_a));
         is.push_back(new LoadInstruction(out, &rv_a));
         break;
         
      case word_size: abort();
         
      case long_size:
         /* ld hl,0
          * or a,a
          * sbc hl,<in>
          * adc a,a
          * ld <out>,a
          */
         is.push_back(new LoadInstruction(&rv_hl, &imm_l<0>));
         is.push_back(new OrInstruction(&rv_a, &rv_a));
         is.push_back(new SbcInstruction(&rv_hl, in));
         is.push_back(new AdcInstruction(&rv_a, &rv_a));
         is.push_back(new LoadInstruction(out, &rv_a));
         break;

      default:
         abort();
      }
   }

   Block *emit_incdec(CgenEnv& env, Block *block, bool inc_not_dec, bool pre_not_post,
                      ASTExpr *subexpr, const Value *out) {
      const VariableValue *lval = new VariableValue(long_size);
      block = subexpr->CodeGen(env, block, lval, ASTExpr::ExprKind::EXPR_LVALUE);
      Instructions& is = block->instrs();
      int bytes = subexpr->type()->bytes();

      /* preincrement if necessary */
      if (pre_not_post) {
         switch (bytes) {
         case byte_size:
            /* ld a,(<lval>)
             * inc/dec a
             * ld (<lval>),a
             * ld <out>,a
             * NOTE: Target for peephole optimization.
             */
            {
               const MemoryValue *memval = new MemoryValue(lval, subexpr->type()->bytes());
               is.push_back(new LoadInstruction(&rv_a, memval));
               if (inc_not_dec) { is.push_back(new IncInstruction(&rv_a)); }
               else { is.push_back(new DecInstruction(&rv_a)); }
               is.push_back(new LoadInstruction(memval, &rv_a));
               if (out) { is.push_back(new LoadInstruction(out, &rv_a)); }
            }
            break;
         case word_size: abort();
         case long_size:
            /* ld hl,<lval>
             * ld <rval>,(hl)
             * inc/dec <rval>
             * ld (hl),<rval>
             * ld <out>,<rval>
             */
            {
               const VariableValue *rval = new VariableValue(long_size);
               const MemoryValue *memval = new MemoryValue(&rv_hl, long_size);
               is.push_back(new LoadInstruction(&rv_hl, lval));
               is.push_back(new LoadInstruction(rval, memval));
               if (inc_not_dec) { is.push_back(new IncInstruction(rval)); }
               else { is.push_back(new DecInstruction(rval)); }
               is.push_back(new LoadInstruction(memval, rval));
               if (out) { is.push_back(new LoadInstruction(out, rval)); }
            }
            break;
         }
      } else {
         /* post inc/dec */
         switch (bytes) {
         case byte_size:
            /* ld a,(<lval>)
             * ld <out>,a
             * inc/dec a
             * ld (<lval>),a
             */
            {
               const MemoryValue *memval = new MemoryValue(lval, byte_size);
               is.push_back(new LoadInstruction(&rv_a, memval));
               is.push_back(new LoadInstruction(out, &rv_a));
               if (inc_not_dec) { is.push_back(new IncInstruction(&rv_a)); }
               else { is.push_back(new DecInstruction(&rv_a)); }
               is.push_back(new LoadInstruction(memval, &rv_a));
            }
            break;
         case word_size: abort();
         case long_size:
            /* ld hl,<lval>
             * ld <rval>,(hl)
             * ld <out>,<rval>
             * inc/dec <rval>
             * ld (hl),<rval>
             */
            {
               const VariableValue *rval = new VariableValue(long_size);
               const MemoryValue *memval = new MemoryValue(&rv_hl, long_size);
               is.push_back(new LoadInstruction(&rv_hl, lval));
               is.push_back(new LoadInstruction(rval, memval));
               is.push_back(new LoadInstruction(out, rval));
               if (inc_not_dec) { is.push_back(new IncInstruction(rval)); }
               else { is.push_back(new DecInstruction(rval)); }
               is.push_back(new LoadInstruction(memval, rval));
            }
            break;
         }
      }

      return block;
   }

   
   
   Block *emit_binop(CgenEnv& env, Block *block, ASTBinaryExpr *expr, const Value *out_lhs,
                     const Value *out_rhs) {
      Instructions& instrs = block->instrs();

      /* evaluate lhs */
      block = expr->lhs()->CodeGen(env, block, out_lhs, ASTBinaryExpr::ExprKind::EXPR_RVALUE);
      
      /* evaluate rhs */
      block = expr->rhs()->CodeGen(env, block, out_rhs, ASTBinaryExpr::ExprKind::EXPR_RVALUE);

      return block;
   }

   void emit_crt(const std::string& name, Block *block) {
      const LabelValue *lv = g_crt.val(name);
      block->instrs().push_back(new CallInstruction(lv));
   }


   void emit_frameset(CgenEnv& env, Block *block) {
      /* push ix
       * ld ix,-<locals>
       * add ix,sp
       * ld sp,ix
       */
      int locals_bytes = env.ext_env().frame().locals_bytes();
      std::vector<Instruction *> frameset
         {new PushInstruction(&rv_ix),
          new LoadInstruction(&rv_ix, new ImmediateValue(-locals_bytes, long_size)),
          new AddInstruction(&rv_ix, &rv_sp),
          new LoadInstruction(&rv_sp, &rv_ix),
         };
      block->instrs().insert(block->instrs().begin(), frameset.begin(), frameset.end());
   }

   void emit_frameunset(CgenEnv& env, Block *block) {
      /* lea ix,ix+locals_bytes
       * pop ix
       * ret
       */
      block->instrs().push_back(new LeaInstruction
                                (&rv_ix, new IndexedRegisterValue
                                 (&rv_ix, env.ext_env().frame().locals_bytes())));
      block->instrs().push_back(new LoadInstruction(&rv_sp, &rv_ix));
      block->instrs().push_back(new PopInstruction(&rv_ix));
      block->instrs().push_back(new RetInstruction());
   }

   /*** Other ***/
   void ByteRegister::Cast(Block *block, const Register *from) const {
      switch (from->kind()) {
      case Kind::REG_BYTE:
         break;
      case Kind::REG_MULTIBYTE:
         {
            const MultibyteRegister *from_mb = dynamic_cast<const MultibyteRegister *>(from);
            const ByteRegister *from_lsb = from_mb->regs().back();
            const RegisterValue *from_rv = new RegisterValue(from_lsb);
            block->instrs().push_back(new LoadInstruction(new RegisterValue(this), from_rv));
         }
         break;
      }
   }

   void MultibyteRegister::Cast(Block *block, const Register *from) const {
      switch (from->kind()) {
      case Kind::REG_BYTE:
         /* ensure long register being cast to does not contain given register. */
         assert(!contains(from));

         block->instrs().push_back(new LoadInstruction(new RegisterValue(this), &imm_l<0>));
         block->instrs().push_back(new LoadInstruction(new RegisterValue(regs()[1]),
                                                       new RegisterValue(from)));

         break;
         
      case Kind::REG_MULTIBYTE:
         break;
      }
   }
   
   /*** ASM DUMPS ***/
   void CgenEnv::DumpAsm(std::ostream& os) const {
      /* dump function implementations */
      impls().DumpAsm(os);

      /* dump string constants */
      strconsts().DumpAsm(os);
   }

   void FunctionImpls::DumpAsm(std::ostream& os) const {
      for (const FunctionImpl& impl : impls()) {
         impl.DumpAsm(os);
      }
   }

   void StringConstants::DumpAsm(std::ostream& os) const {
      for (auto it : strs_) {
         /* emit label */
         it.second->EmitDef(os);

         /* emit string */
         os << "\t.db\t\"" << it.first << "\",0" << std::endl;
      }
   }

   void Block::DumpAsm(const Block *block, std::ostream& os, const FunctionImpl *impl) {
      /* emit label */
      block->label()->EmitDef(os);
      
      /* emit instructions */
      for (const Instruction *instr : block->instrs()) {
         instr->Emit(os);
      }

      /* emit transitions */
      for (const BlockTransition *trans : block->transitions().vec()) {
         trans->DumpAsm(os, impl);
      }
   }
   
   void FunctionImpl::DumpAsm(std::ostream& os) const {
      Blocks visited;

      void (*fn)(const Block *, std::ostream& os, const FunctionImpl *) = Block::DumpAsm;
      
      entry()->for_each_block(visited, fn, os, this);
      fin()->for_each_block(visited, fn, os, this);      
   }

   void JumpTransition::DumpAsm(std::ostream& os, const FunctionImpl *impl) const {
      os << "\tjp\t";
      switch (cond()) {
      case Cond::Z:
         os << "z,";
         break;
      case Cond::NZ:
         os << "nz,";
         break;
      case Cond::C:
         os << "c,";
         break;
      case Cond::NC:
         os << "nc,";
         break;
      case Cond::ANY:
         break;
      }

      dst()->label()->EmitRef(os);

      os << std::endl;
   }

   void ReturnTransition::DumpAsm(std::ostream& os, const FunctionImpl *impl) const
   {
      os << "\tjp\t";
      switch (cond()) {
      case Cond::Z:
         os << "z,";
         break;
      case Cond::NZ:
         os << "nz,";
         break;
      case Cond::C:
         os << "c,";
         break;
      case Cond::NC:
         os << "nc,";
         break;
      case Cond::ANY:
         break;
      }

      impl->fin()->label()->EmitRef(os);

      os << std::endl;
   }   
   

   /*** FRAME GEN & STACK FRAME ***/

   StackFrame::StackFrame(): base_bytes_(long_size * 2), locals_bytes_(0), args_bytes_(0) {}
   
   StackFrame::StackFrame(const VarDeclarations *params):
      base_bytes_(long_size * 2), locals_bytes_(0), args_bytes_(0), next_local_addr_(nullptr),
      next_arg_addr_(nullptr) /* saved FP, RA */ {
      /* add size for each param */
      args_bytes_ = params->size() * long_size;
   }

   int StackFrame::bytes() const { return base_bytes_ + locals_bytes_ + args_bytes_; }
   
   void StackFrame::add_local(const VarDeclaration *local) { add_local(local->bytes()); }

   VarSymInfo *StackFrame::next_arg(const VarDeclaration *arg) {
      if (next_arg_addr_ == nullptr) {
         next_arg_addr_ = FP_idxval.Add(locals_bytes_ + base_bytes_);
      }
      VarSymInfo *info = new VarSymInfo(next_arg_addr_, arg);
       next_arg_addr_ = next_arg_addr_->Add(long_size);
       return info;
    }

    VarSymInfo *StackFrame::next_local(const VarDeclaration *decl) {
       if (next_local_addr_ == nullptr) {
          next_local_addr_ = &FP_idxval;
       }

       VarSymInfo *info = new VarSymInfo(next_local_addr_, decl);
       next_local_addr_ = next_local_addr_->Add(decl->type()->bytes());
       return info;
   }


    void FunctionDef::FrameGen(StackFrame& frame) const {
       comp_stat()->FrameGen(frame);
    }
 
   void CompoundStat::FrameGen(StackFrame& frame) const {
      for (const Declaration *decl : *decls()) {
         decl->FrameGen(frame);
         for (const ASTStat *stat : stats()->vec()) {
            stat->FrameGen(frame);
         }
      }
   }

   void IfStat::FrameGen(StackFrame& frame) const {
      if_body()->FrameGen(frame);
      if (else_body()) {
         else_body()->FrameGen(frame);
      }
   }

   void WhileStat::FrameGen(StackFrame& frame) const {
      body()->FrameGen(frame);
   }

   void ForStat::FrameGen(StackFrame& frame) const {
      body()->FrameGen(frame);
   }

   void LoopStat::FrameGen(StackFrame& frame) const {
      body()->FrameGen(frame);
   }

   void VarDeclaration::FrameGen(StackFrame& frame) const {
      frame.add_local(this);
   }

   void VarDeclaration::Declare(CgenEnv& env) {
      SymInfo *info = env.ext_env().frame().next_local(this);
      env.symtab().AddToScope(sym(), info);
   }

   void TypeDeclaration::Declare(CgenEnv& env) {
      dynamic_cast<DeclarableType *>(type())->Declare(env);
   }

   void Enumerator::Declare(CgenEnv& env) {
      const Value *val = new ImmediateValue(eval(), enum_type()->bytes());
      
      env.symtab().AddToScope
         (sym(),
          new ConstSymInfo(val, VarDeclaration::Create(sym(), true, enum_type(), loc())));
          
   }
   
   

   void EnumType::Declare(CgenEnv& env) {
      if (membs()) {
         for (auto memb : *membs()) {
            memb->Declare(env);
         }
      }
   }


   int PointerType::bytes() const {
      return long_size;
   }

   int FunctionType::bytes() const {
      return long_size; /* actually treated as pointer */
   }

   int VoidType::bytes() const {
      return 0;
   }

   int IntegralType::bytes() const {
      using Kind = IntegralType::IntKind;
      switch (int_kind()) {
      case Kind::SPEC_CHAR: return byte_size;
      case Kind::SPEC_SHORT: return word_size;
      case Kind::SPEC_INT:
      case Kind::SPEC_LONG:
      case Kind::SPEC_LONG_LONG:
         return long_size;
      }
   }

   int StructType::bytes() const {
      return std::accumulate(membs()->begin(), membs()->end(), 0,
                             [](int acc, auto it) -> int {
                                return acc + it->bytes();
                             });
   }

   int UnionType::bytes() const {
      std::vector<int> sizes;
      std::transform(membs()->begin(), membs()->end(), std::back_inserter(sizes),
                     [](auto it) { return it->bytes(); });
      return *std::max_element(sizes.begin(), sizes.end());
   }

   int ArrayType::bytes() const {
      return elem()->bytes() * int_count();
   }

   int StructType::offset(const Symbol *sym) const {
      auto it = std::find_if(membs()->begin(), membs()->end(),
                             [&](auto it) -> bool {
                                return it->sym() == sym;
                             });
      return std::accumulate(membs()->begin(), it, 0,
                             [](int acc, auto it) -> int {
                                return acc + it->bytes();
                             });
   }

}
