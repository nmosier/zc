#include <ostream>

#include "asm.hpp"

namespace zc::z80 {

   void Instruction::Emit(std::ostream& os) const {
      os << "\t" << name() << "\t" << std::endl;
   }

   template <Size sz>
   void BinaryInstruction<sz>::Emit(std::ostream& os) const {
      os << "\t" << name() << "\t";
      dst()->Emit(os);
      os << ",";
      src()->Emit(os);
      os << std::endl;
   }

   template <Size sz>
   void UnaryInstruction<sz>::Emit(std::ostream& os) const {
      os << "\t" << name() << "\t";
      dst()->Emit(os);
      os << std::endl;
   }

   void JumpCondInstruction::Emit(std::ostream& os) const {
      os << "\t" << name() << "\t";
      cond()->Emit(os);
      os << ",";
      dst()->Emit(os);
      os << std::endl;
   }

   void RetCondInstruction::Emit(std::ostream& os) const {
      os << "\t" << name() << "\t";
      cond()->Emit(os);
      os << std::endl;
   }
   
}
