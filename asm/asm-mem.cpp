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

}
