#include <cassert>

#include "asm.hpp"

namespace zc::z80 {

   void Label::EmitRef(std::ostream& os) const {
      os << name();
   }

   void Label::EmitDef(std::ostream& os) const {
      Label::EmitRef(os);
      os << ":" << std::endl;
   }

   void MemoryLocation::Emit(std::ostream& os) const {
      addr()->Emit(os);
   }

}
