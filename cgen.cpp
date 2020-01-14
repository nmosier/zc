#include <numeric>
#include <string>
#include <unordered_set>

#include "ast.hpp"
#include "asm.hpp"

#include "cgen.hpp"

namespace zc {

   static Block dead_block;
   
   void Cgen(TranslationUnit *root, std::ostream& os, const char *filename) {
      CgenEnv env;
      root->CodeGen(env);
      
      env.DumpAsm(os);
   }

   FunctionImpl::FunctionImpl(const CgenEnv& env, const Block *entry, const Block *fin):
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

      if (type->kind() == ASTType::Kind::TYPE_FUNCTION) {
         /* Functions _always_ behave as lvalues, i.e. they are always treated as addresses. */
         addr_ = val_ = label_val;
      } else {
         addr_ = label_val;

         MemoryLocation *mem_loc = new MemoryLocation(label_val);
         val_ = new MemoryValue(mem_loc, type->bytes());
      }
   }

   VarSymInfo::VarSymInfo(const Value *addr, const VarDeclaration *decl): SymInfo(), addr_(addr) {
      auto loc = new MemoryLocation(addr);
      auto val = new MemoryValue(loc, decl->bytes());
      val_ = val;
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

#if 0
   void ASTType::CodeGen(CgenEnv& env) {
      SymInfo *info = env.ext_env().frame().next_local(this);

      if (sym() != nullptr) {
         /* if this type is bounded to a symbol, declare that symbol */
         env.symtab().AddToScope(sym(), info);
      }
   }
#endif

#if 0
   void EnumType::CodeGen(CgenEnv& env) {
      for (auto memb : *membs()) {
         const auto imm = new ImmediateValue(memb->eval(), bytes());
         auto info = new ConstSymInfo(this, imm);
         env.symtab().AddToScope(memb->sym(), info);
      }

      ASTType::CodeGen(env);
   }
#endif

   Block *ReturnStat::CodeGen(CgenEnv& env, Block *block) {
      /* generate returned expression 
       * For now, assume result will be in %a or %hl. */
      block = expr()->CodeGen(env, block, ASTExpr::ExprKind::EXPR_RVALUE);
      
      /* append return transition */
      block->transitions().vec().push_back(new ReturnTransition(Cond::ANY));

      /* create new dummy block (this should be removed as dead code with optimization) */
      Block *dead_block = new Block(new_label());
      block->transitions().vec().push_back(new JumpTransition(dead_block, Cond::ANY));
      return dead_block;
   }

   Block *ExprStat::CodeGen(CgenEnv& env, Block *block) {
      return expr()->CodeGen(env, block, ASTExpr::ExprKind::EXPR_RVALUE);
   }

   Block *IfStat::CodeGen(CgenEnv& env, Block *block) {
      /* Evaluate predicate */
      block = cond()->CodeGen(env, block, ASTExpr::ExprKind::EXPR_RVALUE);

      /* Test predicate 
       * NOTE: For now, assume result is in %a or %hl. */
      emit_nonzero_test(env, block, cond()->type()->bytes());

      /* Create joining label and block. */
      Label *join_label = new_label("if_join");
      Block *join_block = new Block(join_label);

      /* Create if, else Blocks. */
      Label *if_label = new_label("if_block");
      Block *if_block = new Block(if_label);

      Label *else_label = new_label("else_block");
      Block *else_block = new Block(else_label);

      /* Create block transitions to if, else blocks */
      BlockTransition *if_transition = new JumpTransition(if_block, Cond::NZ);
      BlockTransition *else_transition = new JumpTransition(else_block, Cond::ANY);

      block->transitions().vec().push_back(if_transition);
      block->transitions().vec().push_back(else_transition);
      
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
      
      pred_block = pred()->CodeGen(env, pred_block, ASTExpr::ExprKind::EXPR_RVALUE);
      emit_nonzero_test(env, block, pred()->type()->bytes());
      pred_block->transitions().vec().push_back(new JumpTransition(body_block, Cond::NZ));
      pred_block->transitions().vec().push_back(new JumpTransition(join_block, Cond::ANY));

      body_block = body()->CodeGen(env, body_block);
      body_block->transitions().vec().push_back(new JumpTransition(pred_block, Cond::ANY));

      block->transitions().vec().push_back(new JumpTransition(pred_block, Cond::ANY));

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

   Block *AssignmentExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      assert(mode == ExprKind::EXPR_RVALUE);

      /* compute right-hand rvalue */
      int bytes = rhs()->type()->bytes();
      block = rhs()->CodeGen(env, block, ExprKind::EXPR_RVALUE);

      /* save right-hand value */
      switch (bytes) {
      case byte_size:
         block->instrs().push_back(new PushInstruction(&rv_af));
         break;
      case word_size:
         abort();
      case long_size:
         block->instrs().push_back(new PushInstruction(&rv_hl));
      }

      /* compute left-hand lvalue */
      block = lhs()->CodeGen(env, block, ExprKind::EXPR_LVALUE);

      /* restore right-hand value */
      switch (bytes) {
      case byte_size:
         block->instrs().push_back(new PopInstruction(new RegisterValue(&r_af)));
         block->instrs().push_back
            (new LoadInstruction
             (new MemoryValue
              (new MemoryLocation
               (new RegisterValue
                (&r_hl)
                ),
               byte_size
               ),
              new RegisterValue
              (&r_a)
              )
             );
         break;
         
      case word_size: throw std::logic_error("not implemented");
         
      case long_size:
         block->instrs().push_back(new PopInstruction(new RegisterValue(&r_de)));
         block->instrs().push_back
            (new LoadInstruction
             (new MemoryValue
              (new MemoryLocation
               (new RegisterValue
                (&r_hl)
                ),
               long_size
               ),
              new RegisterValue
              (&r_de)
              )
             );
         break;
      }

      return block;
   }

   Block *UnaryExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      int bytes = type()->bytes();
      switch (kind()) {
      case Kind::UOP_ADDR:
         assert(mode == ExprKind::EXPR_RVALUE);
         
         /* get subexpression as lvalue */
         block = expr()->CodeGen(env, block, ExprKind::EXPR_LVALUE);
         break;
         
      case Kind::UOP_DEREFERENCE:
         /* get address as rvalue */
         block = expr()->CodeGen(env, block, ExprKind::EXPR_RVALUE);

         switch (mode) {
         case ExprKind::EXPR_RVALUE:
            switch (bytes) {
            case byte_size:
               block->instrs().push_back
                  (new LoadInstruction
                   (&rv_a,
                    new MemoryValue
                    (new MemoryLocation(&rv_hl),
                     byte_size)));
               break;
            case word_size: abort();
            case long_size:
               block->instrs().push_back
                  (new LoadInstruction
                   (&rv_hl,
                    new MemoryValue
                    (new MemoryLocation(&rv_hl),
                     long_size)));
               break;
            }
            break;
            
         case ExprKind::EXPR_LVALUE:
            break;
         case ExprKind::EXPR_NONE: abort();
         }

         break;
         
      case Kind::UOP_POSITIVE:
         /* nop */
         break;
         
      case Kind::UOP_NEGATIVE:
         switch (bytes) {
         case byte_size:
            block->instrs().push_back(new NegInstruction());
            break;
         case word_size: abort();
         case long_size:
            block->instrs().push_back(new LoadInstruction(&rv_de, &imm_l<0>));
            block->instrs().push_back(new ExInstruction(&rv_de, &rv_hl));
            block->instrs().push_back(new XorInstruction(&rv_a, &rv_a));
            block->instrs().push_back(new SbcInstruction(&rv_hl, &rv_de));
            break;
         }
         break;
         
      case Kind::UOP_BITWISE_NOT:
         switch (bytes) {
         case byte_size:
            block->instrs().push_back(new CplInstruction());
            break;
         case word_size: abort();
         case long_size:
            /* NOTE: this uses the property of 2's complement that it is 1's complement plus 1. */
            block->instrs().push_back(new LoadInstruction(&rv_de, &imm_l<0>));
            block->instrs().push_back(new ExInstruction(&rv_de, &rv_hl));
            block->instrs().push_back(new ScfInstruction());
            block->instrs().push_back(new SbcInstruction(&rv_hl, &rv_de));
            break;
         }
         break;
         
      case Kind::UOP_LOGICAL_NOT:
         emit_logical_not(env, block, bytes);
         break;
      }

      return block;
   }

   Block *BinaryExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      switch (kind()) {
      case Kind::BOP_LOGICAL_AND:
         /* Short-circuit evaluation dictates that evaluation stops if the first operand 
          * is 0. 
          */
         {
            /* evaluate lhs */
            block = lhs()->CodeGen(env, block, ExprKind::EXPR_RVALUE);
            emit_booleanize(env, block, lhs()->type()->bytes());
            emit_nonzero_test(env, block, byte_size); /* NOTE: This should be optimized 
                                                       * away in the future. */

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
            cont_block = rhs()->CodeGen(env, cont_block, ExprKind::EXPR_RVALUE);
            emit_booleanize(env, block, rhs()->type()->bytes());
            cont_block->transitions().vec().push_back(end_transition);

            return end_block;
         }
         
      case Kind::BOP_LOGICAL_OR:
         /* Short-circuit evaluation dictates that evaluation continues until an operand is nonzero.
          */
         {
            /* evaluate lhs */
            block = lhs()->CodeGen(env, block, ExprKind::EXPR_RVALUE);
            emit_booleanize(env, block, lhs()->type()->bytes());
            emit_nonzero_test(env, block, byte_size); /* NOTE: This should be optimized 
                                                       * away in the future. */

            /* create transitions */
            const Label *end_label = new_label("BOP_LOGICAL_OR");
            Block *end_block = new Block(end_label);
            BlockTransition *end_transition = new JumpTransition(end_block, Cond::NZ);
            block->transitions().vec().push_back(end_transition);
            
            const Label *cont_label = new_label("BOP_LOGICAL_AND");
            Block *cont_block = new Block(cont_label);
            BlockTransition *cont_transition = new JumpTransition(end_block, Cond::Z);
            block->transitions().vec().push_back(cont_transition);

            /* Evaluate rhs */
            cont_block = rhs()->CodeGen(env, cont_block, ExprKind::EXPR_RVALUE);
            emit_booleanize(env, block, rhs()->type()->bytes());
            cont_block->transitions().vec().push_back(end_transition);
            
            return end_block;            
         }

      case Kind::BOP_EQ:
      case Kind::BOP_NEQ:
         emit_binop(env, block, this);
         switch (lhs()->type()->bytes()) {
         case byte_size:
            block->instrs().push_back(new CompInstruction(&rv_a, &rv_b));
            break;
         case word_size: abort();
         case long_size:
            block->instrs().push_back(new XorInstruction(&rv_a, &rv_a));
            block->instrs().push_back(new SbcInstruction(&rv_hl, &rv_de));
            break;
         default:
            abort();
         }
         block = emit_ld_a_zf(env, block, kind() == Kind::BOP_NEQ);
         break;

      case Kind::BOP_LT:
         emit_binop(env, block, this);
         switch (lhs()->type()->bytes()) {
         case byte_size:
            /* cp a,b
             * ld a,0
             * adc a,a
             */
            block->instrs().push_back(new CompInstruction(&rv_a, &rv_b));
            block->instrs().push_back(new LoadInstruction(&rv_a, &imm_b<0>));
            break;
                                  
         case word_size: abort();
         case long_size:
            /* xor a,a
             * sbc hl,de
             * adc a,a
             */
            block->instrs().push_back(new XorInstruction(&rv_a, &rv_a));
            block->instrs().push_back(new SbcInstruction(&rv_hl, &rv_de));
            break;
         default: abort();
         }
                               
         block->instrs().push_back(new AdcInstruction(&rv_a, &rv_a));
         break;

      case Kind::BOP_LEQ:
         emit_binop(env, block, this);
         switch (lhs()->type()->bytes()) {
         case byte_size:
            /* scf
             * sbc a,b
             * ld a,0
             * adc a,a
             */
            block->instrs().push_back(new ScfInstruction());
            block->instrs().push_back(new SbcInstruction(&rv_a, &rv_b));
            block->instrs().push_back(new LoadInstruction(&rv_a, &imm_b<0>));
            break;

         case word_size: abort();
         case long_size:
            /* xor a,a
             * scf
             * sbc hl,de
             * adc a,a
             */
            block->instrs().push_back(new XorInstruction(&rv_a, &rv_a));
            block->instrs().push_back(new ScfInstruction());
            block->instrs().push_back(new SbcInstruction(&rv_hl, &rv_de));
            break;
         }

         block->instrs().push_back(new AdcInstruction(&rv_a, &rv_a));
         break;

      case Kind::BOP_GT:
         emit_binop(env, block, this);
         switch (lhs()->type()->bytes()) {
         case byte_size:
            /* dec a
             * cp a,b
             * ld a,1
             * sbc a,0
             */
            block->instrs().push_back(new DecInstruction(&rv_a));
            block->instrs().push_back(new CompInstruction(&rv_a, &rv_b));
            block->instrs().push_back(new LoadInstruction(&rv_a, &imm_b<1>));
            block->instrs().push_back(new SbcInstruction(&rv_a, &imm_b<0>));
            break;

         case word_size: abort();
         case long_size:
            /* dec hl
             * xor a,a
             * sbc hl,de
             * sbc a,a
             * inc a
             */
            block->instrs().push_back(new DecInstruction(&rv_hl));
            block->instrs().push_back(new XorInstruction(&rv_a, &rv_a));
            block->instrs().push_back(new SbcInstruction(&rv_hl, &rv_de));
            block->instrs().push_back(new SbcInstruction(&rv_a, &rv_a));
            block->instrs().push_back(new IncInstruction(&rv_a));
            break;
         }
         break;

      case Kind::BOP_GEQ:
         emit_binop(env, block, this);
         switch (lhs()->type()->bytes()) {
         case byte_size:
            /* cp a,b
             * ld a,1
             * sbc a,0
             */
            block->instrs().push_back(new CompInstruction(&rv_a, &rv_b));
            break;

         case word_size: abort();

         case long_size:
            /* xor a,a
             * sbc hl,de
             * ld a,1
             * sbc a,0
             */
            block->instrs().push_back(new XorInstruction(&rv_a, &rv_a));
            block->instrs().push_back(new SbcInstruction(&rv_hl, &rv_de));
            break;
         }
                               
         block->instrs().push_back(new LoadInstruction(&rv_a, &imm_b<1>));
         block->instrs().push_back(new SbcInstruction(&rv_a, &imm_b<0>));
         break;

      case Kind::BOP_PLUS:
         emit_binop(env, block, this);
         switch (type()->bytes()) {
         case byte_size:
            /* add a,b */
            block->instrs().push_back(new AddInstruction(&rv_a, &rv_b));
            break;
         case word_size: abort();
         case long_size:
            /* add hl,de */
            block->instrs().push_back(new AddInstruction(&rv_hl, &rv_de));
            break;
         }
         break;

      case Kind::BOP_MINUS:
         emit_binop(env, block, this);
         switch (type()->bytes()) {
         case byte_size:
            /* sub a,b */
            block->instrs().push_back(new SubInstruction(&rv_a, &rv_b));
            break;
         case word_size: abort();
         case long_size:
            /* or a,a
             * sbc hl,de 
             */
            block->instrs().push_back(new OrInstruction(&rv_a, &rv_a));
            block->instrs().push_back(new SbcInstruction(&rv_hl, &rv_de));
            break;
         }
         break;

      case Kind::BOP_TIMES:
         emit_binop(env, block, this);
         switch (type()->bytes()) {
         case byte_size:
            /* ld c,a
             * mlt bc
             * ld a,c
             */
            block->instrs().push_back(new LoadInstruction(&rv_c, &rv_a));
            block->instrs().push_back(new MultInstruction(&rv_bc));
            block->instrs().push_back(new LoadInstruction(&rv_a, &rv_c));
            break;
         case word_size:
         case long_size:
            abort();
         }
         break;

      case Kind::BOP_DIVIDE:
      case Kind::BOP_MOD:
      case Kind::BOP_BITWISE_AND:
      case Kind::BOP_BITWISE_OR:
      case Kind::BOP_BITWISE_XOR:
         /* TODO */
         abort();
      }

      return block;
   }

   Block *LiteralExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      const RegisterValue *rv;
      switch (type()->bytes()) {
      case byte_size:
         rv = &rv_a;
         break;
      case word_size: abort();
      case long_size:
         rv = &rv_hl;
         break;
      }
      const ImmediateValue *imm = new ImmediateValue(val(), type()->bytes());

      block->instrs().push_back(new LoadInstruction(rv, imm));
      return block;
   }
   
   Block *StringExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      assert(mode == ExprKind::EXPR_RVALUE); /* string constants aren't assignable */
      block->instrs().push_back(new LoadInstruction(&rv_hl, env.strconsts().Ref(*str())));
      return block;
   }


   
   Block *IdentifierExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      const SymInfo *id_info = env.symtab().Lookup(id()->id());

      switch (mode) {
      case ExprKind::EXPR_NONE: abort();
      case ExprKind::EXPR_LVALUE:
         {
            /* obtain address of identifier */
            const Value *id_addr = dynamic_cast<const VarSymInfo *>(id_info)->addr();
            block->instrs().push_back(new LeaInstruction(&rv_hl, id_addr));
         }
         break;
         
      case ExprKind::EXPR_RVALUE:
         {
            const Value *id_rval = id_info->val();
            const RegisterValue *rv;
            switch (type()->bytes()) {
            case byte_size:
               rv = &rv_a;
               break;
            case word_size: abort();
            case long_size:
               rv = &rv_hl;
               break;
            default: abort();
            }
            block->instrs().push_back(new LoadInstruction(rv, id_rval));
         }
         break;
      }

      return block;
   }

   Block *CallExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      /* push arguments onto stack (1st arg is highest on stack) */

      /* codegen params */
      for (ASTExpr *param : params()->vec()) {
         block = param->CodeGen(env, block, ExprKind::EXPR_RVALUE);
         const RegisterValue *rv;
         switch (param->type()->bytes()) {
         case byte_size:
            rv = &rv_af;
            break;
         case word_size:
         case long_size:
            rv = &rv_hl;
            break;
         }
         block->instrs().push_back(new PushInstruction(rv));
      }

      /* codegen callee */
      block = fn()->CodeGen(env, block, ExprKind::EXPR_RVALUE);
      
      /* call fn */
      block->instrs().push_back(new PushInstruction(&rv_hl));
      block->instrs().push_back(new PopInstruction(&rv_iy));
      block->instrs().push_back(new CallInstruction(&crt_lv_indcall));

      /* pop off args */
      for (ASTExpr *param : params()->vec()) {
         /* TODO: this could be optimized. */
         block->instrs().push_back(new PopInstruction(&rv_de)); /* pop off into scrap register */
      }
      
      return block;
   }

   Block *CastExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      assert(mode == ExprKind::EXPR_RVALUE);

      const Register *src = return_register(expr()->type()->bytes());
      const Register *dst = return_register(type()->bytes());

      expr()->CodeGen(env, block, mode);
      dst->Cast(block, src);

      return block;
   }

   Block *MembExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      /* code generate struct as lvalue */
      block = expr()->CodeGen(env, block, ExprKind::EXPR_LVALUE);
      
      /* get internal offset */
      int offset = dynamic_cast<CompoundType *>(expr()->type())->offset(memb());
      ImmediateValue *imm = new ImmediateValue(offset, long_size);
      block->instrs().push_back(new LoadInstruction(&rv_de, imm));
      block->instrs().push_back(new AddInstruction(&rv_hl, &rv_de));
      
      switch (mode) {
      case ExprKind::EXPR_LVALUE:
         break; /* already computed address */
         
      case ExprKind::EXPR_RVALUE:
         {
            const RegisterValue *dst;
         
            switch (type()->bytes()) {
            case byte_size:
               dst = &rv_a;
               break;
            case word_size: abort();
            case long_size:
               dst = &rv_hl;
               break;
            default:
               abort();
            }
         
            auto memloc = new MemoryLocation(dst);
            auto memval = new MemoryValue(memloc, type()->bytes());
            block->instrs().push_back(new LoadInstruction(&rv_hl, memval));
         }
         break;

      case ExprKind::EXPR_NONE:
         abort();
      }

      return block;
   }

   Block *SizeofExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      struct CodeGenVisitor {
         int operator()(const ASTType *type) { return type->bytes(); }
         int operator()(const ASTExpr *expr) { return expr->type()->bytes(); }
      };

      int bytes = std::visit(CodeGenVisitor(), variant_);
      block->instrs().push_back(new LoadInstruction(&rv_hl, new ImmediateValue(bytes, long_size)));
      return block;
   }

   Block *IndexExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      block = index()->CodeGen(env, block, ExprKind::EXPR_RVALUE);
      block->instrs().push_back
         (new LoadInstruction
          (&rv_bc,
           new ImmediateValue(type()->bytes(), long_size)));
      emit_crt("__imuls", block);
      block->instrs().push_back(new PushInstruction(&rv_hl));
      block = base()->CodeGen(env, block, ExprKind::EXPR_LVALUE);
      block->instrs().push_back(new PopInstruction(&rv_de));
      block->instrs().push_back(new AddInstruction(&rv_hl, &rv_de));

      switch (mode) {
      case ExprKind::EXPR_LVALUE:
         break;
         
      case ExprKind::EXPR_RVALUE:
         {
            MemoryValue *val = new MemoryValue(new MemoryLocation(&rv_hl), type()->bytes());
            block->instrs().push_back(new LoadInstruction
                                      (new RegisterValue(return_register(type()->bytes())), val));
         }
         break;
      case ExprKind::EXPR_NONE: abort();
      }
      return block;
   }


   
   /*** OTHER ***/

   static int label_counter = 0;

   const Register *return_register(int bytes) {
      switch (bytes) {
      case byte_size:
         return &r_a;
      case word_size:
      case long_size:
         return &r_hl;
      default:
         abort();
      }
   }

   Label *new_label() { return new_label(std::string()); }
   Label *new_label(const std::string& prefix) {
      std::string s = std::string("__LABEL_") + prefix + "_" + std::to_string(label_counter++);
      return new Label(s);
   }

   void emit_nonzero_test(CgenEnv& env, Block *block, int bytes) {
      Block::InstrVec& instrs = block->instrs();
      
      switch (bytes) {
      case byte_size:
         /* or a,a */
         instrs.push_back(new OrInstruction(new RegisterValue(&r_a),
                                            new RegisterValue(&r_a)));
         break;
         
      case word_size:
      case long_size:
         /* ld de,0
          * xor a,a
          * sbc hl,de
          */
         instrs.push_back(new LoadInstruction(new RegisterValue(&r_de),
                                              new ImmediateValue(0, long_size)));
         instrs.push_back(new XorInstruction(new RegisterValue(&r_a), new RegisterValue(&r_a)));
         instrs.push_back(new SbcInstruction(new RegisterValue(&r_hl), new RegisterValue(&r_de)));
         break;
      }
   }

   void emit_logical_not(CgenEnv& env, Block *block, int bytes) {
      Block::InstrVec& instrs = block->instrs();
      
      switch (bytes) {
      case byte_size:
         /* cp a,1   ; CF set iff a == 0
          * ld a,0   ; 
          * adc a,a  ; a <- 1 if CF set; a <- 0 if CF reset
          */
         instrs.push_back(new CompInstruction(&rv_a, &imm_b<1>));
         instrs.push_back(new LoadInstruction(&rv_a, &imm_b<0>));
         instrs.push_back(new AdcInstruction(&rv_a, &rv_a)); /* save one byte */
         break;

      case word_size: abort();
         
      case long_size:
         /* ld de,0
          * scf 
          * sbc hl,de
          * ex de,hl
          * adc hl,hl
          */
         instrs.push_back(new LoadInstruction(&rv_de, &imm_l<0>));
         instrs.push_back(new ScfInstruction());
         instrs.push_back(new SbcInstruction(&rv_hl, &rv_de));
         instrs.push_back(new ExInstruction(&rv_de, &rv_hl));
         instrs.push_back(new AdcInstruction(&rv_hl, &rv_hl));
         break;
      }
   }

   void emit_booleanize(CgenEnv& env, Block *block, int bytes) {
      Block::InstrVec& instrs = block->instrs();

      switch (bytes) {
      case byte_size:
         /* neg
          * ld a,0
          * adc a,a
          */
         instrs.push_back(new NegInstruction());
         instrs.push_back(new LoadInstruction(&rv_a, &imm_b<0>));
         instrs.push_back(new AdcInstruction(&rv_a, &rv_a));
         break;
         
      case word_size: abort();
         
      case long_size:
         /* ex de,hl
          * ld hl,0
          * xor a,a
          * sbc hl,de
          * adc a,a
          */
         instrs.push_back(new ExInstruction(&rv_de, &rv_hl));
         instrs.push_back(new LoadInstruction(&rv_hl, &imm_l<0>));
         instrs.push_back(new XorInstruction(&rv_a, &rv_a));
         instrs.push_back(new SbcInstruction(&rv_hl, &rv_de));
         instrs.push_back(new AdcInstruction(&rv_a, &rv_a));
         break;

      default:
         abort();
      }
   }

   void emit_binop(CgenEnv& env, Block *block, ASTBinaryExpr *expr) {
      Block::InstrVec& instrs = block->instrs();
      int bytes = expr->type()->bytes();
      int lhs_bytes = expr->lhs()->type()->bytes();
      int rhs_bytes = expr->rhs()->type()->bytes();

      /* evaluate rhs first */
      block = expr->rhs()->CodeGen(env, block, ASTBinaryExpr::ExprKind::EXPR_RVALUE);

      /* save rhs result */
      switch (rhs_bytes) {
      case byte_size:
         block->instrs().push_back(new PushInstruction(&rv_af));
         break;
      case word_size: abort();
      case long_size:
         block->instrs().push_back(new PushInstruction(&rv_hl));
         break;
      default:
         abort();
      }

      /* evaluate lhs */
      block = expr->lhs()->CodeGen(env, block, ASTBinaryExpr::ExprKind::EXPR_RVALUE);

      /* restore rhs result */
      switch (rhs_bytes) {
      case byte_size:
         block->instrs().push_back(new PopInstruction(&rv_bc));
         break;
      case word_size: abort();
      case long_size:
         block->instrs().push_back(new PopInstruction(&rv_de));
         break;
      default: abort();
      }
   }

   void emit_crt(const std::string& name, Block *block) {
      const LabelValue *lv = crt.val(name);
      block->instrs().push_back(new CallInstruction(lv));
   }

   Block *emit_ld_a_zf(CgenEnv& env, Block *block, bool inverted) {
      block->instrs().push_back(new LoadInstruction(&rv_a, &imm_b<0>));
      
      const Label *end_label = new_label();
      Block *end_block = new Block(end_label);
      BlockTransition *end_transition = new JumpTransition(end_block,
                                                           inverted ? Cond::Z : Cond::NZ);
      const Label *cont_label = new_label();
      Block *cont_block = new Block(cont_label);
      BlockTransition *cont_transition = new JumpTransition(cont_block,
                                                            inverted ? Cond::NZ : Cond::Z);
      
      block->transitions().vec().push_back(end_transition);
      block->transitions().vec().push_back(cont_transition);
      
      cont_block->instrs().push_back(new IncInstruction(&rv_a));

      /* join */
      cont_block->transitions().vec().push_back(new JumpTransition(end_block, Cond::ANY));

      return end_block;
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
      block->instrs().push_back
         (new LeaInstruction
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

   void FunctionImpl::DumpAsm(std::ostream& os) const {
      std::unordered_set<const Block *> emitted_blocks;
      entry()->DumpAsm(os, emitted_blocks, this);
      fin()->DumpAsm(os, emitted_blocks, this);
   }
   
   void Block::DumpAsm(std::ostream& os,
                       std::unordered_set<const Block *>& emitted_blocks,
                       const FunctionImpl *impl) const {
      /* check if already emitted */
      if (emitted_blocks.find(this) != emitted_blocks.end()) {
         return;
      } else {
         emitted_blocks.insert(this);
      }
      
      label()->EmitDef(os);

      for (const Instruction *instr : instrs()) {
         instr->Emit(os);
      }

      /* emit transitions */
      std::unordered_set<const Block *> dsts;
      transitions().DumpAsm(os, dsts, impl);

      /* emit destination blocks */
      for (const Block *block : dsts) {
         block->DumpAsm(os, emitted_blocks, impl);
      }
   }

   void BlockTransitions::DumpAsm(std::ostream& os,
                                  std::unordered_set<const Block *>& to_emit,
                                  const FunctionImpl *impl) const {
      for (const BlockTransition *trans : vec()) {
         trans->DumpAsm(os, to_emit, impl);
      }
   }

   void JumpTransition::DumpAsm(std::ostream& os,
                                std::unordered_set<const Block *>& to_emit,
                                const FunctionImpl *impl) const {
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

      to_emit.insert(dst());
   }

   void ReturnTransition::DumpAsm(std::ostream& os,
                                  std::unordered_set<const Block *>& to_emit,
                                  const FunctionImpl *impl) const {
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
       next_local_addr_ = next_local_addr_->Add(decl->bytes());
       return info;
    }
   
    void FunctionDef::FrameGen(StackFrame& frame) const {
       comp_stat()->FrameGen(frame);
    }

    void CompoundStat::FrameGen(StackFrame& frame) const {
       for (const Declaration *decl : *decls()) {
          decl->FrameGen(frame);
       }
      for (const ASTStat *stat : stats()->vec()) {
         stat->FrameGen(frame);
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
          new ConstSymInfo(VarDeclaration::Create(sym(), true, enum_type(), loc()),
                               val));
          
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
