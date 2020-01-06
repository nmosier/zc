#include <stdexcept>

#include "asm.hpp"

namespace zc::z80 {

   /*** EMIT ***/
   
   void ImmediateValue::Emit(std::ostream& os) const {
      os << imm_;
   }

   void LabelValue::Emit(std::ostream& os) const {
      label()->EmitRef(os);
   }

   void RegisterValue::Emit(std::ostream& os) const {
      reg()->Emit(os);
   }

   void IndexedRegisterValue::Emit(std::ostream& os) const {
      val()->Emit(os);
      os << "+";
      os << (int) index(); 
   }

   void OffsetValue::Emit(std::ostream& os) const {
      base()->Emit(os);
      os << "+";
      os << offset();
   }

   void MemoryValue::Emit(std::ostream& os) const {
      os << "(";
      loc()->Emit(os);
      os << ")";
   }

   /*** ADD ***/
   
   Value *ImmediateValue::Add(const intmax_t& offset) const {
      return new ImmediateValue(imm() + offset, size());
   }

   Value *LabelValue::Add(const intmax_t& offset) const {
      return new OffsetValue(this, offset, size());
   }

   Value *RegisterValue::Add(const intmax_t& offset) const {
      return new IndexedRegisterValue(this, offset);
   }

   Value *IndexedRegisterValue::Add(const intmax_t& offset) const {
      return new IndexedRegisterValue(val(), index() + offset);
   }

   Value *OffsetValue::Add(const intmax_t& new_offset) const {
      return new OffsetValue(base(), offset() + new_offset, size());
   }

   Value *MemoryValue::Add(const intmax_t& offset) const {
      throw std::logic_error("attempted to offset a memory value");
   }

   MemoryLocation *MemoryLocation::Advance(const intmax_t& offset) const {
      return new MemoryLocation(addr()->Add(offset));
   }
      
   MemoryValue *MemoryValue::Next(int next_size) const {
      return new MemoryValue(loc()->Advance(size()), next_size);
   }

   MemoryValue *MemoryValue::Prev(int next_size) const {
      return new MemoryValue(loc()->Advance(-next_size), next_size);
   }

}
