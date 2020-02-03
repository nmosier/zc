#include "asm.hpp"

namespace zc::z80 {

#if 0
#if Z80
   bool ez80_mode = true;
#elif EZ80
   bool ez80_mode = false;
#endif


   int flag_size = 0;
   int byte_size = 1;
   int word_size = 2;
#if Z80
   int long_size = 2;
#elif EZ80
   int long_size = 3;
#endif
#endif
   
}
