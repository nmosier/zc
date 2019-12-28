#ifndef __ASM_HPP
#error "include \"asm.hpp\""
#endif

#ifndef __ASM_REG_HPP
#define __ASM_REG_HPP

#include <array>

#include "asm/asm-mode.hpp"
#include "asm/asm-val.hpp"

namespace zc::z80 {

   class ByteRegister;
   class MultibyteRegister;

   extern const ByteRegister r_a, r_b, r_c, r_d, r_e, r_f, r_h, r_l, r_ixh, r_ixl, r_iyh, r_iyl;
   extern const MultibyteRegister r_af, r_bc, r_de, r_hl, r_ix, r_iy;
   
   /*************
    * REGISTERS *
    *************/

   /**
    * Base class representing a register (single- or multi-byte registers).
    */
   class Register {
   public:
      const std::string& name() const { return name_; }
      Size size() const { return size_; }

      Register(const std::string& name, Size size): name_(name), size_(size) {}
      
   protected:
      const std::string name_;
      Size size_;
   };

   class ByteRegister: public Register {
   public:
      template <typename... Args> ByteRegister(Args... args): Register(args..., Size::BYTE) {}
      
   protected:
   };

   class MultibyteRegister: public Register {
   public:
      typedef std::array<const ByteRegister *, word_size> ByteRegs;
      const ByteRegs& regs() const { return regs_; }

      template <typename... Args>
      MultibyteRegister(const ByteRegs& regs, Args... args):
         Register(args..., Size::WORD), regs_(regs) {}
      
   protected:
      const ByteRegs regs_;
   };

   

}

#endif
