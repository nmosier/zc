#ifndef __ASM_HPP
#error "include \"asm.hpp\""
#endif

#ifndef __ASM_MODE_HPP
#define __ASM_MODE_HPP

namespace zc::z80 {

   constexpr bool ez80_mode =
#ifdef Z80
      false
#else
      true
#endif
      ;
   
   constexpr int byte_size = 1;
   constexpr int word_size = 2;
   constexpr int long_size =
#ifdef Z80
      2
#else
      3
#endif
      ;
   
}

#endif
