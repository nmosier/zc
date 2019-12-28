
#ifndef __ASM_HPP
#error "include \"asm.hpp\", not \"asm-base.hpp\" directly"
#endif

#ifndef __ASM_BASE_HPP
#define __ASM_BASE_HPP

namespace zc {

   class ASMInstruction {
   public:

      /** 
       * Number of cycles required to execute the instruction.
       */
      virtual int cycles() const = 0;

   protected:
   };

   
}

#endif
