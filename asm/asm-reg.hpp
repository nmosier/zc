#ifndef __ASM_HPP
#error "include \"asm.hpp\""
#endif

#ifndef __ASM_REG_HPP
#define __ASM_REG_HPP

#include <array>

#include "asm/asm-mode.hpp"
#include "asm/asm-val.hpp"

namespace zc::z80 {

   extern const Register<Size::BYTE> r_a, r_b, r_c, r_d, r_e, r_f, r_h, r_l, r_ixh,
      r_ixl, r_iyh, r_iyl;
   extern const Register<Size::LONG> r_af, r_bc, r_de, r_hl, r_ix, r_iy, r_sp;
   
   /*************
    * REGISTERS *
    *************/

   /**
    * Base class representing a register (single- or multi-byte registers).
    */
   class Register_ {
   public:
      const std::string& name() const { return name_; }
      const Size size() const { return size_; }

      Register_(const std::string& name, Size size): name_(name), size_(size) {}
      
   protected:
      const std::string name_;
      Size size_;
   };

   template <Size sz>
   class Register: public Register_ {
   public:
      template <typename... Args> Register(Args... args): Register_(args..., sz) {}
      
   protected:
   };

   template <>
   class Register<Size::LONG>: public Register_ {
   public:
      typedef std::array<const Register<Size::BYTE> *, word_size> ByteRegs;
      const ByteRegs& regs() const { return regs_; }
      
      template <typename... Args>
      Register<Size::LONG>(const ByteRegs& regs, Args... args):
         Register_(args..., Size::LONG), regs_(regs) {}
      
   protected:
      const ByteRegs regs_;
   };

}

#endif
