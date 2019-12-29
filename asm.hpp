#ifndef __ASM_HPP
#define __ASM_HPP

namespace zc::z80 {

#ifndef Z80
#ifndef EZ80
#error "Z80 or EZ80 flag must be defined at compile time"
#endif
#endif

   enum class Size {BYTE, WORD, LONG};
   
}

#include "asm/asm-instr.hpp"
#include "asm/asm-mem.hpp"
#include "asm/asm-reg.hpp"
#include "asm/asm-val.hpp"
#include "asm/asm-mode.hpp"
#include "asm/asm-cond.hpp"

#endif
