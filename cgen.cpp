#include <numeric>
#include <string>

#include "ast.hpp"
#include "asm.hpp"

#include "cgen.hpp"

namespace zc {

   FunctionImpl::FunctionImpl(const Block *entry): entry_(entry) {
      addr_ = new LabelValue(entry->label());
   }

   BlockTransitions::BlockTransitions(const Transitions& vec): vec_(vec) {
      /* get mask of conditions */
      /* TODO: this doesn't work. */
   }

   void CgenExtEnv::Enter(Symbol *sym) {
      sym_env_.Enter(sym);
      next_local_ = &fp_memval;
   }

   void CgenExtEnv::Exit() {
      sym_env_.Exit();
      next_local_ = nullptr;
   }


   SymInfo::SymInfo(const ExternalDecl *ext_decl) {
      type_ = ext_decl->Type();
      Label *label = new Label(std::string("_") + *ext_decl->sym());
      LabelValue *label_val = new LabelValue(label);

      if (type_->kind() == ASTType::Kind::TYPE_FUNCTION) {
         val_ = label_val;
      } else {
         MemoryLocation *mem_loc = new MemoryLocation(label_val);
         val_ = new MemoryValue(mem_loc, bytes(type_));
      }
   }

   const MemoryValue *CgenExtEnv::NewLocal(Size size) {
      return (next_local_ = next_local_->Prev(bytes(size)));
   }

   /*** CODE GENERATION ***/

   void TranslationUnit::CodeGen(CgenEnv& env) {
      env.symtab().EnterScope();

      for (ExternalDecl *decl : decls()->vec()) {
         decl->CodeGen(env);
      }
      
      env.symtab().ExitScope();
   }

   void ExternalDecl::CodeGen(CgenEnv& env) {
      /* enter global variable into global scope */
      SymInfo *info = new SymInfo(this);
      env.symtab().AddToScope(sym(), info);
   }

   void FunctionDef::CodeGen(CgenEnv& env) {
      /* enter function into global scope */
      SymInfo *info = new SymInfo(this);
      env.symtab().AddToScope(sym(), info);
      env.ext_env().Enter(sym());
      
      /* enter parameters into subscope */
      env.symtab().EnterScope();
      
      const MemoryLocation *argloc = FP_loc.Advance(long_size); /* advance past return address */
      for (ASTType *type : Type()->get_callable()->params()->vec()) {
         Value *argval = new MemoryValue(argloc, bytes(type));
         SymInfo *info = new SymInfo(type, argval);
         env.symtab().AddToScope(type->sym(), info);
         argloc = argloc->Advance(long_size);
      }
      
      Block *start_block = new Block(dynamic_cast<const LabelValue *>(info->val())->label());
      
      Block *end_block = comp_stat()->CodeGen(env, start_block, false);
      
      FunctionImpl impl(start_block);


      env.symtab().ExitScope();
      env.ext_env().Exit();
   }

   Block *CompoundStat::CodeGen(CgenEnv& env, Block *block, bool new_scope) {
      if (new_scope) {
         env.symtab().EnterScope();
      }

      for (Decl *decl : decls()->vec()) {
         decl->CodeGen(env);
      }

      for (ASTStat *stat : stats()->vec()) {
         block = stat->CodeGen(env, block);
      }
      
      if (new_scope) {
         env.symtab().ExitScope();
      }

      return block;
   }

   void Decl::CodeGen(CgenEnv& env) {
      const MemoryValue *val = env.ext_env().NewLocal(Type()->size());

      /* add decl to scope */
      SymInfo *info = new SymInfo(Type(), val);
      env.symtab().AddToScope(sym(), info);
   }

   Block *ReturnStat::CodeGen(CgenEnv& env, Block *block) {
      // const ASTType *type = env.ext_env().Type();
      // Size size = type->size();
      // const Register *reg = return_register(sz);

      /* generate returned expression 
       * For now, assume result will be in %a or %hl. */
      block = expr()->CodeGen(env, block);
      
      /* append return transition */
      block->transitions().vec().push_back(new ReturnTransition(Cond::ANY));
      // block->instrs().push_back(RetInstruction());

      return nullptr; /* nullptr signifies that this block has no outgoing transisitons */
   }

   Block *ExprStat::CodeGen(CgenEnv& env, Block *block) {
      return expr()->CodeGen(env, block);
   }

   Block *IfStat::CodeGen(CgenEnv& env, Block *block) {
      /* Evaluate predicate */
      block = cond()->CodeGen(env, block);

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

   Block *AssignmentExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      assert(mode == ExprKind::EXPR_RVALUE);

      /* compute right-hand rvalue */
      block = rhs()->CodeGen(env, block, ExprKind::EXPR_RVALUE);

      /* save right-hand value */
      block->instrs().push_back(PushInstruction(new RegisterValue(return_register(size))));

      /* compute left-hand lvalue */
      block = lhs()->CodeGen(env, block, ExprKind::EXPR_LVALUE);

      /* restore right-hand value */
      switch (bytes(size)) {
      case byte_size:
         block->instrs().push_back(PopInstruction(new RegisterValue(&r_af)));
         block->instrs().push_back
            (LoadInstruction
             (new MemoryValue
              (new MemoryLocation
               (new RegisterValue
                (&r_hl)
                )
               ),
              new RegisterValue
              (&r_a)
              )
             );
         break;
         
      case word_size: throw std::logic_error("not implemented");
         
      case long_size:
         block->instrs().push_back(PopInstruction(new RegisterValue(&r_de)));
         block->instrs().push_back
            (LoadInstruction
             (new MemoryValue
              (new MemoryLocation
               (new RegisterValue
                (&r_hl)
                )
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
      switch (kind()) {
      case Kind::UOP_ADDR:
         /* get subexpression as lvalue */
         block = expr()->CodeGen(env, block, ExprKind::EXPR_LVALUE);
         break;
         
      case Kind::UOP_DEREFERENCE:
         /* get subexpression as rvalue */
         block = expr()->CodeGen(env, block, ExprKind::EXPR_RVALUE);

         /* load pointer from resulting address */
         block->instrs().push_back(LoadInstruction(&rv_hl, &rv_hl));
         
         break;
         
      case Kind::UOP_POSITIVE:
         /* nop */
         break;
         
      case Kind::UOP_NEGATIVE:
         switch (bytes(type()->size())) {
         case byte_size:
            block->instrs().push_back(NegInstruction());
            break;
         case word_size: abort();
         case long_size:
            block->instrs().push_back(LoadInstruction(&rv_de, &imm_0_l));
            block->instrs().push_back(ExInstruction(&rv_de, &rv_hl));
            block->instrs().push_back(XorInstruction(&rv_a, &rv_a));
            block->instrs().push_back(SbcInstruction(&rv_hl, &rv_de));
            break;
         }
         break;
         
      case Kind::UOP_BITWISE_NOT:
         switch (bytes(type()->size())) {
         case byte_size:
            block->instrs().push_back(CplInstruction());
            break;
         case word_size: abort();
         case long_size:
            /* NOTE: this uses the property of 2's complement that it is 1's complement plus 1. */
            block->instrs().push_back(LoadInstruction(&rv_de, &imm_0_l));
            block->instrs().push_back(ExInstruction(&rv_de, &rv_hl));
            block->instrs().push_back(ScfInstruction());
            block->instrs().push_back(SbcInstruction(&rv_hl, &rv_de));
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
      Size size = type()->size();
      int bytes = bytes(size);

      switch (kind()) {
      case Kind::BOP_LOGICAL_AND:
         /* Short-circuit evaluation dictates that evaluation stops if the first operand 
          * is 0. 
          */
         {
            /* evaluate lhs */
            block = lhs()->CodeGen(env, block, ExprKind::EXPR_RVALUE);
            emit_booleanize(env, block, size);
            emit_nonzero_test(env, block, size); /* NOTE: This should be optimized 
                                                  * away in the future. */

            /* create transitions */
            const Label *end_label = new_label("BOP_LOGICAL_AND");
            Block *end_block = new Block(end_label);
            BlockTransition *end_transition = new JumpTransition(end_block, Cond::Z);
            block->transitions().push_back(end_transition);

            const Label *cont_label = new_label("BOP_LOGICAL_AND");
            Block *cont_block = new Block(cont_label);
            BlockTransition *cont_transition = new JumpTransition(end_block, Cond::NZ);
            block->transitions().push_back(cont_transition);

            /* Evaluate rhs */
            cont_block = rhs()->CodeGen(env, cont_block, ExprKind::EXPR_RVALUE);
            emit_booleanize(env, block, size);
            cont_block->transitions().push_back(end_transition);

            return end_block;
         }
         
      case Kind::BOP_LOGICAL_OR:
         /* Short-circuit evaluation dictates that evaluation continues until an operand is nonzero.
          */
         {
            /* evaluate lhs */
            block = lhs()->CodeGen(env, block, ExprKind::EXPR_RVALUE);
            emit_booleanize(env, block, size);
            emit_nonzero_test(env, block, size); /* NOTE: This should be optimized 
                                                  * away in the future. */

            /* create transitions */
            const Label *end_label = new_label("BOP_LOGICAL_OR");
            Block *end_block = new Block(end_label);
            BlockTransition *end_transition = new JumpTransition(end_block, Cond::NZ);
            block->transitions().push_back(end_transition);
            
            const Label *cont_label = new_label("BOP_LOGICAL_AND");
            Block *cont_block = new Block(cont_label);
            BlockTransition *cont_transition = new JumpTransition(end_block, Cond::Z);
            block->transitions().push_back(cont_transition);

            /* Evaluate rhs */
            cont_block = rhs()->CodeGen(env, cont_block, ExprKind::EXPR_RVALUE);
            emit_booleanize(env, block, size);
            cont_block->transitions().push_back(end_transition);
            
            return end_block;            
         }

      case Kind::BOP_EQ:
      case Kind::BOP_NEQ:
         block = emit_binop(env, block, size,
                            [](CgenEnv& env, Block *block, Size size) -> Block * {
                               switch (bytes(size)) {
                               case byte_size:
                                  block->instrs().push_back(CompInstruction(&rv_a, &rv_b));
                                  break;
                               case word_size: abort();
                               case long_size:
                                  block->instrs().push_back(XorInstruction(&rv_a, &rv_a));
                                  block->instrs().push_back(SbcInstruction(&rv_hl, &rv_de));
                                  break;
                               }
                               
                               return (block = emit_ld_a_zf(env, block, kind() == Kind::BOP_NEQ));
                            });
         break;

      case Kind::BOP_LT:
         block = emit_binop(env, block, size,
                            [](CgenEnv& env, Block *block, Size size) -> Block * {
                               switch (bytes(size)) {
                               case byte_size:
                                  /* cp a,b
                                   * ld a,0
                                   * adc a,a
                                   */
                                  block->instrs().push_back(CompInstruction(&rv_a, &rv_b));
                                  block->instrs().push_back(LoadInstruction(&rv_a, &imm_b<0>));
                                  break;
                                  
                               case word_size: abort();
                               case long_size:
                                  /* xor a,a
                                   * sbc hl,de
                                   * adc a,a
                                   */
                                  block->instrs().push_back(XorInstruction(&rv_a, &rv_a));
                                  block->instrs().push_back(SbcInstruction(&rv_hl, &rv_de));
                                  break;
                               }
                               
                               block->instrs().push_back(AdcInstruction(&rv_a, &rv_a));
                               return block;
                            });
         break;

      case Kind::BOP_LEQ:
         block = emit_binop(env, block, size,
                            [](CgenEnv& env, Block *block, Size size) -> Block * {
                               switch (bytes(size)) {
                               case byte_size:
                                  /* scf
                                   * sbc a,b
                                   * ld a,0
                                   * adc a,a
                                   */
                                  block->instrs().push_back(ScfInstruction());
                                  block->instrs().push_back(SbcInstruction(&rv_a, &rv_b));
                                  block->instrs().push_back(LoadInstruction(&rv_a, &imm_b<0>));
                                  break;

                               case word_size: abort();
                               case long_size:
                                  /* xor a,a
                                   * scf
                                   * sbc hl,de
                                   * adc a,a
                                   */
                                  block->instrs().push_back(XorInstruction(&rv_a, &rv_a));
                                  block->instrs().push_back(ScfInstruction());
                                  block->instrs().push_back(SbcInstruction(&rv_hl, &rv_de));
                                  break;
                               }

                               block->instrs().push_back(AdcInstruction(&rv_a, &rv_a));
                               return block;
                            });
         break;

      case BOP_GT:
         block = emit_binop(env, block, size,
                            [](CgenEnv& env, Block *block, Size size) -> Block * {
                               switch (bytes(size)) {
                               case byte_size:
                                  /* dec a
                                   * cp a,b
                                   * ld a,1
                                   * sbc a,0
                                   */
                                  block->instrs().push_back(DecInstruction(&rv_a));
                                  block->instrs().push_back(CompInstruction(&rv_a, &rv_b));
                                  block->instrs().push_back(LoadInstruction(&rv_a, &imm_b<1>));
                                  block->instrs().push_back(SbcInstruction(&rv_a, &imm_b<0>));
                                  break;

                               case word_size: abort();
                               case long_size:
                                  /* dec hl
                                   * xor a,a
                                   * sbc hl,de
                                   * sbc a,a
                                   * inc a
                                   */
                                  block->instrs().push_back(DecInstruction(&rv_hl));
                                  block->instrs().push_back(XorInstruction(&rv_a, &rv_a));
                                  block->instrs().push_back(SbcInstruction(&rv_hl, &rv_de));
                                  block->instrs().push_back(SbcInstruction(&rv_a, &rb_a));
                                  block->instrs().push_back(IncInstruction(&rv_a));
                                  break;
                               }

                               return block;
                            });
         break;

         /* TODO */
      }
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
         instrs.push_back(OrInstruction(new RegisterValue(&r_a),
                                        new RegisterValue(&r_a)));
         break;
         
      case word_size:
      case long_size:
         /* ld de,0
          * xor a,a
          * sbc hl,de
          */
         instrs.push_back(LoadInstruction(new RegisterValue(&r_de),
                                          new ImmediateValue(0, long_size)));
         instrs.push_back(XorInstruction(new RegisterValue(&r_a),
                                         new RegisterValue(&r_a)));
         instrs.push_back(SbcInstruction(new RegisterValue(&r_hl),
                                         new RegisterValue(&r_de)));
         break;
      }
   }

   void emit_logical_not(CgenEnv& env, Block *block, Size size) {
      Block::InstrVec& instrs = block->instrs();
      
      switch (size) {
      case byte_size:
         /* cp a,1   ; CF set iff a == 0
          * ld a,0   ; 
          * adc a,a  ; a <- 1 if CF set; a <- 0 if CF reset
          */
         instrs.push_back(CompInstruction(&rv_a, &imm_b<1>));
         instrs.push_back(LoadInstruction(&rv_a, &imm_b<0>));
         instrs.push_back(AdcInstruction(&rv_a, &rv_a)); /* save one byte */
         break;

      case word_size: abort();
         
      case long_size:
         /* ld de,0
          * scf 
          * sbc hl,de
          * ex de,hl
          * adc hl,hl
          */
         instrs.push_back(LoadInstruction(&rv_de, &imm_l<0>));
         instrs.push_back(ScfInstruction());
         instrs.push_back(SbcInstruction(&rv_hl, &rv_de));
         instrs.push_back(ExInstruction(&rv_de, &rv_hl));
         instrs.push_back(AdcInstruction(&rv_hl, &rv_hl));
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
         instrs.push_back(NegInstruction());
         instrs.push_back(LoadInstruction(&rv_a, &imm_b<0>));
         instrs.push_back(AdcInstruction(&rv_a, &rv_a));
         break;
         
      case word_size: abort();
         
      case long_size:
         /* ex de,hl
          * ld hl,0
          * xor a,a
          * sbc hl,de
          * adc a,a
          */
         instrs.push_back(ExInstruction(&rv_de, &rv_hl));
         instrs.push_back(LoadInstruction(&rv_hl, &imm_l<0>));
         instrs.push_back(XorInstruction(&rv_a, &rv_a));
         instrs.push_back(SbcInstruction(&rv_hl, &rv_de));
         instrs.push_back(AdcInstruction(&rv_a, &rv_a));
         break;
      }
   }

   Block *emit_binop(CgenEnv& env, Block *block, Size size,
                     Block *(*op)(CgenEnv& env, Block *block, Size size)) {
      Block::InstrVec& instrs = block->instrs();

      /* evaluate lhs */
      block = lhs()->CodeGen(env, block, ExprKind::EXPR_RVALUE);

      /* save lhs result */
      switch (bytes(type()->size())) {
      case byte_size:
         block->instrs().push_back(PushInstruction(&rv_af));
         break;
      case word_size: abort();
      case long_size:
         block->instrs().push_back(PushInstruction(&rv_hl));
         break;
      }

      /* evaluate rhs */
      block = rhs()->CodeGen(env, block, ExprKind::EXPR_RVALUE);

      /* restore lhs result */
      switch (bytes(type()->size())) {
      case byte_size:
         block->instrs().push_back(PopInstruction(&rv_bc));
         break;
      case word_size: abort();
      case long_size:
         block->instrs().push_back(PopInstruction(&rv_de));
         break;
      }
      
      return op(env, block, size);
   }

   Block *emit_ld_a_zf(CgenEnv& env, Block *block, bool inverted) {
      block->instrs().push_back(LoadInstruction(&rv_a, &imm_b<0>));
      
      const Label *end_label = new_label();
      Block *end_block = new Block(end_label);
      BlockTransition *end_transition = new JumpTransition(end_block,
                                                           inverted ? Cond::Z : Cond::NZ);
      const Label *cont_label = new_label();
      Block *cont_block = new Block(cont_label);
      BlockTransition *cont_transition = new JumpTransition(cont_block,
                                                            inverted ? Cond::NZ : Cond::Z);
      
      block->transitions().push_back(end_transition);
      block->transitions().push_back(cont_transition);

      cont_block->instrs().push_back(IncInstruction(&rv_a));

      /* join */
      cont_block->transitions().push_back(new JumpTransition(end_block, Cond::ANY));

      return end_block;
   }
   
}
