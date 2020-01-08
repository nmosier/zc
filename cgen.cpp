#include <numeric>
#include <string>
#include <unordered_set>

#include "ast.hpp"
#include "asm.hpp"

#include "cgen.hpp"

namespace zc {
   
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
   
   void CgenExtEnv::Enter(Symbol *sym, const Types *args) {
      sym_env_.Enter(sym);
      frame_ = StackFrame(args);
   }

   void CgenExtEnv::Exit() {
      sym_env_.Exit();
   }

   SymInfo::SymInfo(const ExternalDecl *ext_decl) {
      type_ = ext_decl->Type();
      Label *label = new Label(std::string("_") + *ext_decl->sym());
      LabelValue *label_val = new LabelValue(label);

      if (type_->kind() == ASTType::Kind::TYPE_FUNCTION) {
         /* Functions _always_ behave as lvalues, i.e. they are always treated as addresses. */
         addr_ = val_ = label_val;
      } else {
         addr_ = label_val;

         MemoryLocation *mem_loc = new MemoryLocation(label_val);
         val_ = new MemoryValue(mem_loc, bytes(type_));
      }
   }

   SymInfo::SymInfo(const ASTType *type, const Value *val, const Value *addr):
      type_(type), val_(val), addr_(addr) {
      assert(dynamic_cast<const MemoryValue *>(addr) == nullptr);
   }
   
   SymInfo::SymInfo(const ASTType *type, const Value *addr): type_(type), addr_(addr) {
      // assert(dynamic_cast<const MemoryValue *>(addr) == nullptr);
      
      MemoryLocation *mem_loc = new MemoryLocation(addr);
      val_ = new MemoryValue(mem_loc, bytes(type));
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
      /* enter global variable into global scope */
      SymInfo *info = new SymInfo(this);
      env.symtab().AddToScope(sym(), info);
   }

   void FunctionDef::CodeGen(CgenEnv& env) {
      /* enter function into global scope */
      SymInfo *info = new SymInfo(this);
      const Types *args = Type()->get_callable()->params();
      env.symtab().AddToScope(sym(), info);
      env.ext_env().Enter(sym(), args);

      /* enter parameters into subscope */
      env.EnterScope();      

      /* initialize stack frame */
      FrameGen(env.ext_env().frame());

      /* assign argument stack locations */
      for (const ASTType *type : Type()->get_callable()->params()->vec()) {
         SymInfo *info = env.ext_env().frame().next_arg(type);
         env.symtab().AddToScope(type->sym(), info);
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

      for (Decl *decl : decls()->vec()) {
         decl->CodeGen(env);
      }

      for (ASTStat *stat : stats()->vec()) {
         block = stat->CodeGen(env, block);
      }
      
      if (new_scope) {
         env.ExitScope();
      }

      return block;
   }

   void Decl::CodeGen(CgenEnv& env) {
      SymInfo *info = env.ext_env().frame().next_local(Type());

      /* add decl to scope */
      env.symtab().AddToScope(sym(), info);
   }

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
      emit_nonzero_test(env, block, cond()->type()->size());

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
      Block *else_end = else_body()->CodeGen(env, else_block);

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

      BlockTransition *pred_trans = new JumpTransition(pred_block, Cond::ANY);
      block->transitions().vec().push_back(pred_trans);

      pred_block = pred()->CodeGen(env, pred_block, ASTExpr::ExprKind::EXPR_RVALUE);
      emit_nonzero_test(env, block, pred()->type()->size());

      pred_block->transitions().vec().push_back(new JumpTransition(body_block, Cond::NZ));

      body_block = body()->CodeGen(env, body_block);
      body_block->transitions().vec().push_back(pred_trans);

      Label *join_label = new_label("while_end");
      Block *join_block = new Block(join_label);
      pred_block->transitions().vec().push_back(new JumpTransition(join_block, Cond::ANY));

      return join_block;
   }

   Block *AssignmentExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      assert(mode == ExprKind::EXPR_RVALUE);

      /* compute right-hand rvalue */
      Size sz = rhs()->type()->size();
      int bs = bytes(sz);
      block = rhs()->CodeGen(env, block, ExprKind::EXPR_RVALUE);

      /* save right-hand value */
      switch (bs) {
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
      switch (bs) {
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
      int bs = bytes(type()->size());
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
            switch (bs) {
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
         switch (bs) {
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
         switch (bytes(type()->size())) {
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
         emit_logical_not(env, block, type()->size());
         break;
      }

      return block;
   }

   Block *BinaryExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      Size sz = type()->size();
      int bs = bytes(sz);

      /* TODO: this assertion should be removed later once casts are included. */
      assert(lhs()->type()->size() == rhs()->type()->size());
      Size op_sz = lhs()->type()->size();
      int op_bs = bytes(op_sz);
      
      switch (kind()) {
      case Kind::BOP_LOGICAL_AND:
         /* Short-circuit evaluation dictates that evaluation stops if the first operand 
          * is 0. 
          */
         {
            /* evaluate lhs */
            block = lhs()->CodeGen(env, block, ExprKind::EXPR_RVALUE);
            emit_booleanize(env, block, sz);
            emit_nonzero_test(env, block, sz); /* NOTE: This should be optimized 
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
            emit_booleanize(env, block, sz);
            cont_block->transitions().vec().push_back(end_transition);

            return end_block;
         }
         
      case Kind::BOP_LOGICAL_OR:
         /* Short-circuit evaluation dictates that evaluation continues until an operand is nonzero.
          */
         {
            /* evaluate lhs */
            block = lhs()->CodeGen(env, block, ExprKind::EXPR_RVALUE);
            emit_booleanize(env, block, op_sz);
            emit_nonzero_test(env, block, sz); /* NOTE: This should be optimized 
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
            emit_booleanize(env, block, op_sz);
            cont_block->transitions().vec().push_back(end_transition);
            
            return end_block;            
         }

      case Kind::BOP_EQ:
      case Kind::BOP_NEQ:
         emit_binop(env, block, this);
         switch (op_bs) {
         case byte_size:
            block->instrs().push_back(new CompInstruction(&rv_a, &rv_b));
            break;
         case word_size: abort();
         case long_size:
            block->instrs().push_back(new XorInstruction(&rv_a, &rv_a));
            block->instrs().push_back(new SbcInstruction(&rv_hl, &rv_de));
            break;
         }
         block = emit_ld_a_zf(env, block, kind() == Kind::BOP_NEQ);
         break;

      case Kind::BOP_LT:
         emit_binop(env, block, this);
         switch (op_bs) {
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
         }
                               
         block->instrs().push_back(new AdcInstruction(&rv_a, &rv_a));
         break;

      case Kind::BOP_LEQ:
         emit_binop(env, block, this);
         switch (op_bs) {
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
         switch (op_bs) {
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
         switch (op_bs) {
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
         switch (bs) {
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
         switch (bs) {
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
         switch (bs) {
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
      Size sz = type()->size();
      int bs = bytes(sz);
      const RegisterValue *rv;
      switch (bs) {
      case byte_size:
         rv = &rv_a;
         break;
      case word_size: abort();
      case long_size:
         rv = &rv_hl;
         break;
      }
      const ImmediateValue *imm = new ImmediateValue(val(), bs);

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
      Size size = type()->size();      
      
      switch (mode) {
      case ExprKind::EXPR_NONE: abort();
      case ExprKind::EXPR_LVALUE:
         {
            /* obtain address of identifier */
            const Value *id_addr = id_info->addr();
            block->instrs().push_back(new LeaInstruction(&rv_hl, id_addr));
         }
         break;
         
      case ExprKind::EXPR_RVALUE:
         {
            const Value *id_rval = id_info->val();
            const RegisterValue *rv;
            switch (bytes(size)) {
            case byte_size:
               rv = &rv_a;
               break;
            case word_size: abort();
            case long_size:
               rv = &rv_hl;
               break;
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
         switch (bytes(param->type()->size())) {
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

      const Register *src = return_register(expr()->type()->size());
      const Register *dst = return_register(type()->size());

      expr()->CodeGen(env, block, mode);
      dst->Cast(block, src);

      return block;
   }

   /*** OTHER ***/

   static int label_counter = 0;

   const Register *return_register(Size sz) {
      switch (sz) {
      case Size::SZ_CHAR:
         return &r_a;
      case Size::SZ_SHORT:
      case Size::SZ_INT:
      case Size::SZ_LONG:
      case Size::SZ_LONG_LONG:
      case Size::SZ_POINTER:
         return &r_hl;
      }
   }

   Label *new_label() { return new_label(std::string()); }
   Label *new_label(const std::string& prefix) {
      std::string s = std::string("__LABEL_") + prefix + "_" + std::to_string(label_counter++);
      return new Label(s);
   }

   void emit_nonzero_test(CgenEnv& env, Block *block, Size size) {
      Block::InstrVec& instrs = block->instrs();
      
      switch (bytes(size)) {
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

   void emit_logical_not(CgenEnv& env, Block *block, Size size) {
      Block::InstrVec& instrs = block->instrs();
      
      switch (bytes(size)) {
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

   void emit_booleanize(CgenEnv& env, Block *block, Size size) {
      Block::InstrVec& instrs = block->instrs();

      switch (bytes(size)) {
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
      }
   }

   void emit_binop(CgenEnv& env, Block *block, ASTBinaryExpr *expr) {
      Block::InstrVec& instrs = block->instrs();
      Size sz = expr->type()->size();
      int bs = bytes(sz);
      Size op_sz = expr->lhs()->type()->size();
      int op_bs = bytes(op_sz);

      /* evaluate rhs first */
      block = expr->rhs()->CodeGen(env, block, ASTBinaryExpr::ExprKind::EXPR_RVALUE);

      /* save rhs result */
      switch (op_bs) {
      case byte_size:
         block->instrs().push_back(new PushInstruction(&rv_af));
         break;
      case word_size: abort();
      case long_size:
         block->instrs().push_back(new PushInstruction(&rv_hl));
         break;
      }

      /* evaluate lhs */
      block = expr->lhs()->CodeGen(env, block, ASTBinaryExpr::ExprKind::EXPR_RVALUE);

      /* restore rhs result */
      switch (op_bs) {
      case byte_size:
         block->instrs().push_back(new PopInstruction(&rv_bc));
         break;
      case word_size: abort();
      case long_size:
         block->instrs().push_back(new PopInstruction(&rv_de));
         break;
      }
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
   
   StackFrame::StackFrame(const Types *params):
      base_bytes_(long_size * 2), locals_bytes_(0), args_bytes_(0), next_local_addr_(nullptr),
      next_arg_addr_(nullptr) /* saved FP, RA */ {
      /* add size for each param */
      args_bytes_ = params->vec().size() * long_size;
   }

   int StackFrame::bytes() const { return base_bytes_ + locals_bytes_ + args_bytes_; }
   
   void StackFrame::add_local(Size sz) { add_local(::zc::bytes(sz)); }
   void StackFrame::add_local(const ASTType *type) { add_local(type->size()); }

   SymInfo *StackFrame::next_arg(const ASTType *type) {
      if (next_arg_addr_ == nullptr) {
         next_arg_addr_ = FP_idxval.Add(locals_bytes_ + base_bytes_);
      }
      SymInfo *info = new SymInfo(type, next_arg_addr_);
      next_arg_addr_ = next_arg_addr_->Add(long_size);
      return info;
   }

   SymInfo *StackFrame::next_local(const ASTType *type) {
      if (next_local_addr_ == nullptr) {
         next_local_addr_ = &FP_idxval;
      }
      SymInfo *info = new SymInfo(type, next_local_addr_);
      next_local_addr_ = next_local_addr_->Add(type->size());
      return info;
   }
   
   void FunctionDef::FrameGen(StackFrame& frame) const {
      comp_stat()->FrameGen(frame);
   }

   void CompoundStat::FrameGen(StackFrame& frame) const {
      for (const Decl *decl : decls()->vec()) {
         decl->FrameGen(frame);
      }
      for (const ASTStat *stat : stats()->vec()) {
         stat->FrameGen(frame);
      }
   }

   void IfStat::FrameGen(StackFrame& frame) const {
      if_body()->FrameGen(frame);
      else_body()->FrameGen(frame);
   }

   void WhileStat::FrameGen(StackFrame& frame) const {
      body()->FrameGen(frame);
   }

   void Decl::FrameGen(StackFrame& frame) const {
      frame.add_local(Type());
   }



   
   
}
