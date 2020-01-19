#include <stdexcept>

#include "asm.hpp"

namespace zc::z80 {

   /*** EMIT ***/
   
   void ImmediateValue::Emit(std::ostream& os) const {
      os << *imm_;
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


   ImmediateValue::ImmediateValue(const intmax_t& imm): Value_(long_size), imm_(imm) {
      if (imm <= byte_max) {
         size_ = byte_size;
      } else if (imm <= word_max) {
         size_ = word_size;
      } else if (imm <= long_max) {
         size_ = long_size;
      }
   }

   /*** MATCHING ***/
   bool Value::Match(const Value *to) const {
      if (size_ && size() != to->size()) {
         return false;
      } else {
         return Match_(to);
      }
   }

   bool ImmediateValue::Match_aux(const ImmediateValue *to) const {
      if (imm_) {
         return imm() == to->imm();
      } else {
         return true;
      }
   }

   bool RegisterValue::Match_aux(const RegisterValue *to) const {
      if (reg_) {
         return reg()->Eq(to->reg());
      } else {
         return true;
      }
   }

   bool IndexedRegisterValue::Match_aux(const IndexedRegisterValue *to) const {
      if (val_) {
         if (!val()->Match(to->val())) {
            return false;
         }
      }

      if (index_) {
         if (index() != to->index()) {
            return false;
         }
      }

      return true;
   }

   bool MemoryValue::Match_aux(const MemoryValue *to) const {
      if (loc_) {
         return loc()->addr()->Match(to->loc()->addr());
      } else {
         return true;
      }
   }
   
}
