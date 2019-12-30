#include "asm.hpp"

namespace zc::z80 {

   template <Size sz>
   void ImmediateValue<sz>::Emit(std::ostream& os) const {
      os << imm_;
   }

   void LabelValue::Emit(std::ostream& os) const {
      label()->EmitRef(os);
   }

   template <Size sz>
   void RegisterValue<sz>::Emit(std::ostream& os) const {
      reg()->Emit(os);
   }

   template <Size sz>
   void IndexedRegisterValue<sz>::Emit(std::ostream& os) const {
      val()->Emit(os);
      os << "+";
      index()->Emit(os);
   }

   template <Size sz>
   void OffsetValue<sz>::Emit(std::ostream& os) const {
      base()->Emit(os);
      os << "+";
      offset()->Emit(os);
   }

   template <Size sz>
   void MemoryValue<sz>::Emit(std::ostream& os) const {
      os << "(";
      loc()->Emit(os);
      os << ")";
   }
   
}
