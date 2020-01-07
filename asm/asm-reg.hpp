#ifndef __ASM_HPP
#error "include \"asm.hpp\""
#endif

#ifndef __ASM_REG_HPP
#define __ASM_REG_HPP

#include <array>

#include "asm/asm-mode.hpp"

namespace zc::z80 {

   class ByteRegister;
   class MultibyteRegister;
   class RegisterValue;

   extern const ByteRegister r_a, r_b, r_c, r_d, r_e, r_f, r_h, r_l, r_ixh,
      r_ixl, r_iyh, r_iyl;
   extern const MultibyteRegister r_af, r_bc, r_de, r_hl, r_ix, r_iy, r_sp;

   extern const RegisterValue rv_a, rv_b, rv_c, rv_d, rv_e, rv_f, rv_h, rv_l, rv_ixh,
      rv_ixl, rv_iyh, rv_iyl;
   extern const RegisterValue rv_af, rv_bc, rv_de, rv_hl, rv_ix, rv_iy, rv_sp;
   
   /*************
    * REGISTERS *
    *************/

   /**
    * Base class representing a register (single- or multi-byte registers).
    */
   class Register {
   public:
      enum Kind {REG_BYTE, REG_MULTIBYTE};
      virtual Kind kind() const = 0;
      const std::string& name() const { return name_; }
      const int size() const { return size_; }

      void Emit(std::ostream& os) const { os << name(); }
      virtual void Cast(Block *block, const Register *from) const = 0;

   protected:
      const std::string name_;
      int size_;

      Register(const std::string& name, int size): name_(name), size_(size) {}
   };

   class ByteRegister: public Register {
   public:
      virtual Kind kind() const override { return Kind::REG_BYTE; }

      virtual void Cast(Block *block, const Register *from) const override;
      
      template <typename... Args>
      ByteRegister(Args... args): Register(args..., byte_size) {}
      
   protected:
   };

   class MultibyteRegister: public Register {
   public:
      typedef std::array<const ByteRegister *, word_size> ByteRegs;
      virtual Kind kind() const override { return Kind::REG_MULTIBYTE; }
      const ByteRegs& regs() const { return regs_; }
      bool contains(const Register *reg) const;

      virtual void Cast(Block *block, const Register *from) const override;
      
      template <typename... Args>
      MultibyteRegister(const ByteRegs& regs, Args... args):
         Register(args..., long_size), regs_(regs) {}
      
   protected:
      const ByteRegs regs_;
   };

}

#endif
