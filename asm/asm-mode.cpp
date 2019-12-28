#include "asm.hpp"

namespace zc::z80 {

   bool ez80_mode;

   int byte_size() {
      return 1;
   }

   int word_size() {
      return 2;
   }

   int long_size() {
      return ez80_mode ? 3 : 2;
   }
   
}
