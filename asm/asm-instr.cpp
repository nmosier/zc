#include <ostream>

#include "asm.hpp"
#include "cgen.hpp"

namespace zc::z80 {

   void Instruction::Emit(std::ostream& os) const {
      os << "\t" << name();
      if (cond_ || !operands_.empty()) {
         os << "\t";
      }
      if (cond_) {
         os << *cond_;
      }
      for (auto it = operands_.begin(), end = operands_.end(); it != end; ++it) {
         if (it != operands_.begin() || cond_) {
            os << ",";
         }
         (**it)->Emit(os);         
      }
      os << std::endl;
   }

   bool Instruction::Eq(const Instruction *other) const {
      if (name() != other->name()) {
         return false;
      }

      auto it = operands_.begin(), other_it = other->operands_.begin(),
         end = operands_.end(), other_end = other->operands_.end();      
      for (; it != end && other_it != other_end; ++it, ++other_it) {
         if (!(**it)->Eq(**other_it)) {
            return false;
         }
      }

      return it == end && other_it == other_end;
   }

   bool Instruction::Match(const Instruction *other) const {
      if (name() != other->name()) {
         return false;
      }

      auto it = operands_.begin(), other_it = other->operands_.begin(),
         end = operands_.end(), other_end = other->operands_.end();      
      for (; it != end && other_it != other_end; ++it, ++other_it) {
         if (*it) {
            if (!(**it)->Match(**other_it)) {
               return false;
            }
         } else {
            it->send(**other_it);
         }
      }

      return it == end && other_it == other_end;
   }

   /*** GEN & USE ***/

   void BinaryInstruction::Gen(std::list<const Value *>& vals) const { dst()->Gen(vals); }
   void BinaryInstruction::Use(std::list<const Value *>& vals) const {
      dst()->Use(vals); src()->Use(vals);
   }

   void UnaryInstruction::Gen(std::list<const Value *>& vals) const { dst()->Gen(vals); }
   void UnaryInstruction::Use(std::list<const Value *>& vals) const { dst()->Use(vals); }

   void LoadInstruction::Gen(std::list<const Value *>& vals) const { dst()->Gen(vals); }
   void LoadInstruction::Use(std::list<const Value *>& vals) const {
      src()->Use(vals);
      auto mv = dynamic_cast<const MemoryValue *>(dst());
      if (mv) { mv->Use(vals); } /* ugh, this is hideous */
   }

   void BitwiseInstruction::Gen(std::list<const Value *>& vals) const {
      if (!dst()->Eq(src())) { dst()->Gen(vals); }
   }
   void BitwiseInstruction::Use(std::list<const Value *>& vals) const {
      if (!dst()->Eq(src())) { dst()->Use(vals); src()->Use(vals); }
   }

   void CallInstruction::Gen(std::list<const Value *>& vals) const {
      rv_hl.Gen(vals);
      rv_bc.Gen(vals);
      }
   void CallInstruction::Use(std::list<const Value *>& vals) const {
      rv_hl.Use(vals);
      rv_bc.Use(vals);
   }
   
   void CplInstruction::Gen(std::list<const Value *>& vals) const { rv_a.Gen(vals); }
   void CplInstruction::Use(std::list<const Value *>& vals) const { rv_a.Use(vals); }

   void NegInstruction::Gen(std::list<const Value *>& vals) const { rv_a.Gen(vals); }
   void NegInstruction::Use(std::list<const Value *>& vals) const { rv_a.Use(vals); }

   void DjnzInstruction::Gen(std::list<const Value *>& vals) const { rv_b.Gen(vals); }
   void DjnzInstruction::Use(std::list<const Value *>& vals) const { rv_b.Use(vals); }

   void SbcInstruction::Gen(std::list<const Value *>& vals) const { dst()->Gen(vals); }
   void SbcInstruction::Use(std::list<const Value *>& vals) const {
      if (!dst()->Eq(src())) {
         dst()->Gen(vals);
         src()->Gen(vals);
      }
   }
   
   

   /*** VARS ***/
   void Instruction::ReplaceVar(const VariableValue *var, const Value *with) {
      for (Values::iterator operand_it = operands_.begin(), operand_end = operands_.end();
           operand_it != operand_end;
           ++operand_it) {
         *operand_it = (**operand_it)->ReplaceVar(var, with);
      }
   }

   /*** RESOLUTION ***/

   void Instruction::Resolve(Instructions& out) {
      /* resolve all operands first */
      for (auto it = operands().begin(), end = operands().end(); it != end; ++it) {
         *it = (**it)->Resolve();
      }

      /* call auxiliary function */
      Resolve_(out);
   }

   
   void LoadInstruction::Resolve_(Instructions& out) {
      const Register *r1, *r2;
      int sz1, sz2;
      const RegisterValue rv1(&r1, &sz1);
      const RegisterValue rv2(&r2, &sz2);
      const LoadInstruction instr(&rv1, &rv2);
      if (instr.Match(this) && r1 != &r_sp) {
         if (sz1 == sz2 && (sz1 == word_size || sz1 == long_size)) {
            /* push rr1 \ pop rr2 */
            out.push_back(new PushInstruction(src()));
            out.push_back(new PopInstruction(dst()));
            return;
         }
      }

      out.push_back(this);
   }

   
}
