#ifndef __ASM_HPP
#error "include \"asm.hpp\""
#endif

#ifndef __ASM_MODE_HPP
#define __ASM_MODE_HPP

namespace zc::z80 {

   extern bool ez80_mode;
   int byte_size();
   int word_size();
   int long_size();
   
}

#endif
