#include <ostream>

#include "asm.hpp"

namespace zc::z80 {

   void Instruction::Emit(std::ostream& os) const {
      os << "\t" << name();
      if (cond_ || !operands_.empty()) {
         os << "\t";
      }
      if (cond_) {
         cond_->Emit(os);
      }
      for (auto it = operands_.begin(), end = operands_.end(); it != end; ++it) {
         if (it != operands_.begin() || cond_) {
            os << ",";
         }
         (**it)->Emit(os);         
      }
      os << std::endl;
   }

#if 0
   const Value *BinaryInstruction::dst() const { return *operands_.front(); }
   const Value *BinaryInstruction::src() const { return **++operands_.begin(); }
   const Value *UnaryInstruction::dst() const { return *operands_.front(); }
#endif

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
   
   void BitwiseInstruction::Gen(std::list<const Value *>& vals) const {
      if (!dst()->Eq(src())) { vals.push_back(dst()); }
   }

   void BitwiseInstruction::Use(std::list<const Value *>& vals) const {
      if (!dst()->Eq(src())) { vals.push_back(dst()); vals.push_back(src()); }
   }

   void CallInstruction::Gen(std::list<const Value *>& vals) const {
         vals.push_back(&rv_hl); vals.push_back(&rv_bc);
      }
   void CallInstruction::Use(std::list<const Value *>& vals) const {
      vals.push_back(&rv_hl); vals.push_back(&rv_bc);
   }
   
   void CplInstruction::Gen(std::list<const Value *>& vals) const { vals.push_back(&rv_a); }
   void CplInstruction::Use(std::list<const Value *>& vals) const { vals.push_back(&rv_a); }

   void NegInstruction::Gen(std::list<const Value *>& vals) const { vals.push_back(&rv_a); }
   void NegInstruction::Use(std::list<const Value *>& vals) const { vals.push_back(&rv_a); }

   void DjnzInstruction::Gen(std::list<const Value *>& vals) const { vals.push_back(&rv_b); }
   void DjnzInstruction::Use(std::list<const Value *>& vals) const { vals.push_back(&rv_b); }

   /*** VARS ***/
   void Instruction::ReplaceVar(const VariableValue *var, const Value *with) {
      for (Values::iterator operand_it = operands_.begin(), operand_end = operands_.end();
           operand_it != operand_end;
           ++operand_it) {
         *operand_it = (**operand_it)->ReplaceVar(var, with);
      }
   }
}
