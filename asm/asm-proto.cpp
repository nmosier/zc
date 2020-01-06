#include "asm.hpp"

namespace zc::z80 {

   static const RegisterValue FP_register_value(&r_ix);
   const MemoryLocation FP_loc(&FP_register_value);
   const MemoryValue FP_memval(&FP_loc, long_size); /* NOTE: long size because it'll store
                                                     * the return address. */
   const IndexedRegisterValue FP_idxval(&rv_ix, 0);
  
}
