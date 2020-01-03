#include <numeric>
#include <string>

#include "ast.hpp"
#include "asm.hpp"

#include "cgen.hpp"

namespace zc {

   void Cgen(TranslationUnit *root, std::ostream& os, const char *filename) {
      CgenEnv env;
      root->CodeGen(env);
   }
   

   FunctionImpl::FunctionImpl(const Block *entry): entry_(entry) {
      addr_ = new LabelValue(entry->label());
   }

   BlockTransitions::BlockTransitions(const Transitions& vec): vec_(vec) {
      /* get mask of conditions */
      /* TODO: this doesn't work. */
   }

   void CgenExtEnv::Enter(Symbol *sym) {
      sym_env_.Enter(sym);
      next_local_ = &FP_memval;
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
         /* Functions _always_ behave as lvalues, i.e. they are always treated as addresses. */
         lval_ = rval_ = label_val;
      } else {
         lval_ = label_val;

         MemoryLocation *mem_loc = new MemoryLocation(label_val);
         rval_ = new MemoryValue(mem_loc, bytes(type_));
      }
   }

   SymInfo::SymInfo(const ASTType *type, const Value *lval): type_(type), lval_(lval) {
      MemoryLocation *mem_loc = new MemoryLocation(lval);
      rval_ = new MemoryValue(mem_loc, bytes(type_));
   }

   const MemoryValue *CgenExtEnv::NewLocal(Size size) {
      return (next_local_ = next_local_->Prev(bytes(size)));
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
      
      Block *start_block = new Block(dynamic_cast<const LabelValue *>(info->lval())->label());
      
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
      block = expr()->CodeGen(env, block, ASTExpr::ExprKind::EXPR_RVALUE);
      
      /* append return transition */
      block->transitions().vec().push_back(new ReturnTransition(Cond::ANY));
      // block->instrs().push_back(RetInstruction());

      return nullptr; /* nullptr signifies that this block has no outgoing transisitons */
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

   Block *AssignmentExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      assert(mode == ExprKind::EXPR_RVALUE);

      /* compute right-hand rvalue */
      Size sz = rhs()->type()->size();
      int bs = bytes(sz);
      block = rhs()->CodeGen(env, block, ExprKind::EXPR_RVALUE);

      /* save right-hand value */
      block->instrs().push_back
         (PushInstruction
          (new RegisterValue
           (return_register(sz))));

      /* compute left-hand lvalue */
      block = lhs()->CodeGen(env, block, ExprKind::EXPR_LVALUE);

      /* restore right-hand value */
      switch (bs) {
      case byte_size:
         block->instrs().push_back(PopInstruction(new RegisterValue(&r_af)));
         block->instrs().push_back
            (LoadInstruction
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
         block->instrs().push_back(PopInstruction(new RegisterValue(&r_de)));
         block->instrs().push_back
            (LoadInstruction
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
         switch (bs) {
         case byte_size:
            block->instrs().push_back(NegInstruction());
            break;
         case word_size: abort();
         case long_size:
            block->instrs().push_back(LoadInstruction(&rv_de, &imm_l<0>));
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
            block->instrs().push_back(LoadInstruction(&rv_de, &imm_l<0>));
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
      Size sz = type()->size();
      int bs = bytes(sz);

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
            emit_booleanize(env, block, sz);
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
            emit_booleanize(env, block, sz);
            cont_block->transitions().vec().push_back(end_transition);
            
            return end_block;            
         }

      case Kind::BOP_EQ:
      case Kind::BOP_NEQ:
         emit_binop(env, block, this);
         switch (bs) {
         case byte_size:
            block->instrs().push_back(CompInstruction(&rv_a, &rv_b));
            break;
         case word_size: abort();
         case long_size:
            block->instrs().push_back(XorInstruction(&rv_a, &rv_a));
            block->instrs().push_back(SbcInstruction(&rv_hl, &rv_de));
            break;
         }
         block = emit_ld_a_zf(env, block, kind() == Kind::BOP_NEQ);
         break;

      case Kind::BOP_LT:
         emit_binop(env, block, this);
         switch (bs) {
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
         break;

      case Kind::BOP_LEQ:
         emit_binop(env, block, this);
         switch (bs) {
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
         break;

      case Kind::BOP_GT:
         emit_binop(env, block, this);
         switch (bs) {
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
            block->instrs().push_back(SbcInstruction(&rv_a, &rv_a));
            block->instrs().push_back(IncInstruction(&rv_a));
            break;
         }
         break;

      case Kind::BOP_GEQ:
         emit_binop(env, block, this);
         switch (bs) {
         case byte_size:
            /* cp a,b
             * ld a,1
             * sbc a,0
             */
            block->instrs().push_back(CompInstruction(&rv_a, &rv_b));
            break;

         case word_size: abort();

         case long_size:
            /* xor a,a
             * sbc hl,de
             * ld a,1
             * sbc a,0
             */
            block->instrs().push_back(XorInstruction(&rv_a, &rv_a));
            block->instrs().push_back(SbcInstruction(&rv_hl, &rv_de));
            break;
         }
                               
         block->instrs().push_back(LoadInstruction(&rv_a, &imm_b<1>));
         block->instrs().push_back(SbcInstruction(&rv_a, &imm_b<0>));
         break;

      case Kind::BOP_PLUS:
         emit_binop(env, block, this);
         switch (bs) {
         case byte_size:
            /* add a,b */
            block->instrs().push_back(AddInstruction(&rv_a, &rv_b));
            break;
         case word_size: abort();
         case long_size:
            /* add hl,de */
            block->instrs().push_back(AddInstruction(&rv_hl, &rv_de));
            break;
         }
         break;

      case Kind::BOP_MINUS:
         emit_binop(env, block, this);
         switch (bs) {
         case byte_size:
            /* sub a,b */
            block->instrs().push_back(SubInstruction(&rv_a, &rv_b));
            break;
         case word_size: abort();
         case long_size:
            /* or a,a
             * sbc hl,de 
             */
            block->instrs().push_back(OrInstruction(&rv_a, &rv_a));
            block->instrs().push_back(SbcInstruction(&rv_hl, &rv_de));
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
            block->instrs().push_back(LoadInstruction(&rv_c, &rv_a));
            block->instrs().push_back(MultInstruction(&rv_bc));
            block->instrs().push_back(LoadInstruction(&rv_a, &rv_c));
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

      block->instrs().push_back(LoadInstruction(rv, imm));
      return block;
   }

   Block *StringExpr::CodeGen(CgenEnv& env, Block *block, ExprKind mode) {
      assert(mode == ExprKind::EXPR_RVALUE); /* string constants aren't assignable */
      block->instrs().push_back(LoadInstruction(&rv_hl, env.strconsts().Ref(*str())));
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
            const Value *id_lval = id_info->lval();
            block->instrs().push_back(LeaInstruction(&rv_hl, id_lval));
         }
         break;
         
      case ExprKind::EXPR_RVALUE:
         {
            const Value *id_rval = id_info->rval();
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
            block->instrs().push_back(LoadInstruction(rv, id_rval));
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
         block->instrs().push_back(PushInstruction(rv));
      }

      /* codegen callee */
      block = fn()->CodeGen(env, block, ExprKind::EXPR_RVALUE);

      /* call fn */
      block->instrs().push_back(CallInstruction(&crt_lv_call));

      /* pop off args */
      for (ASTExpr *param : params()->vec()) {
         /* TODO: this could be optimized. */
         block->instrs().push_back(PopInstruction(&rv_de)); /* pop off into scrap register */
      }
      
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
         instrs.push_back(XorInstruction(new RegisterValue(&r_a), new RegisterValue(&r_a)));
         instrs.push_back(SbcInstruction(new RegisterValue(&r_hl), new RegisterValue(&r_de)));
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

   void emit_binop(CgenEnv& env, Block *block, ASTBinaryExpr *expr) {
      Block::InstrVec& instrs = block->instrs();
      Size sz = expr->type()->size();
      int bs = bytes(sz);

      /* evaluate lhs */
      block = expr->lhs()->CodeGen(env, block, ASTBinaryExpr::ExprKind::EXPR_RVALUE);

      /* save lhs result */
      switch (bs) {
      case byte_size:
         block->instrs().push_back(PushInstruction(&rv_af));
         break;
      case word_size: abort();
      case long_size:
         block->instrs().push_back(PushInstruction(&rv_hl));
         break;
      }

      /* evaluate rhs */
      block = expr->rhs()->CodeGen(env, block, ASTBinaryExpr::ExprKind::EXPR_RVALUE);

      /* restore lhs result */
      switch (bs) {
      case byte_size:
         block->instrs().push_back(PopInstruction(&rv_bc));
         break;
      case word_size: abort();
      case long_size:
         block->instrs().push_back(PopInstruction(&rv_de));
         break;
      }
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
      
      block->transitions().vec().push_back(end_transition);
      block->transitions().vec().push_back(cont_transition);
      
      cont_block->instrs().push_back(IncInstruction(&rv_a));

      /* join */
      cont_block->transitions().vec().push_back(new JumpTransition(end_block, Cond::ANY));

      return end_block;
   }
   
}
