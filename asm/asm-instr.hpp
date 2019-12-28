
#ifndef __ASM_HPP
#error "include \"asm.hpp\", not \"asm-base.hpp\" directly"
#endif

#ifndef __ASM_BASE_HPP
#define __ASM_BASE_HPP

#include "asm/asm-mem.hpp"

namespace zc::z80 {

   class Instruction {
   public:

      /** 
       * Number of cycles required to execute the instruction.
       */
      virtual int cycles() const = 0;

   protected:
   };

   /***********************
    * INSTRUCTION CLASSES *
    ***********************/

   /**
    * Add instruction class.
    */
   #if 0
   class AddInstruction: public Instruction {
   public:
   protected:
      const Register& dst_;
      
      
   }
#endif
   
}

#endif
