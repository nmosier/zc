#include <cassert>

#include "asm.hpp"

namespace zc::z80 {

   int bytes(Size sz) {
      switch (sz) {
      case Size::BYTE: return byte_size;
      case Size::WORD: return word_size;
      case Size::LONG: return long_size;
      }
   }

   void Label::EmitRef(std::ostream& os) const {
      os << name();
   }

   void Label::EmitDef(std::ostream& os) const {
      Label::EmitRef(os);
      os << ":" << std::endl;
   }

   template <Size sz>
   void MemoryLocation<sz>::Emit(std::ostream& os) const {
      addr()->Emit(os);
   }

}
