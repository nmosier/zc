#ifndef __ASM_HPP
#error "include \"asm.hpp\""
#endif

#ifndef __ASM_REG_HPP
#define __ASM_REG_HPP

#include "asm/asm-val.hpp"

namespace zc::z80 {

   /*************
    * REGISTERS *
    *************/
   
   /**
    * Base class representing a register (single- or multi-byte registers).
    */
   class Register {
   public:
      typedef Value::Kind Kind;
      const std::string& name() const { return name_; }
      Kind kind() const { return kind_; }
      int size() const { return Value::size(kind_); }

      Register(const std::string& name, Kind kind): name_(name), kind_(kind) {}
      
   protected:
      const std::string name_;
      Kind kind_;
   };


   /**
    * Class representing a single-byte register.
    */
   class ByteRegister: public Register {
   public:

      template <typename... Args>
      ByteRegister(Args... args): Register(args..., Kind::V_BYTE) {}

   protected:
   };

   /**
    *  Base class representing a multibyte register.
    */
   class MultibyteRegister: public Register {
   public:
      typedef std::vector<const ByteRegister *> ByteRegs;
      const ByteRegs& regs() const { return regs_; }
      
      template <typename... Args>
      MultibyteRegister(ByteRegs regs_, Args... args);
      
   protected:
      ByteRegs regs_;
   };

   /**
    * Multibyte register that holds a word.
    */
   class RegisterWord: public MultibyteRegister {
   public:
      template <typename... Args>
      RegisterWord(Args... args): MultibyteRegister(args..., Kind::V_WORD) {}

   protected:
   };


   /**
    * Multibyte register that holds a long.
    */
   class RegisterLong: public MultibyteRegister {
   public:

      template <typename... Args>
      RegisterLong(Args... args): Register(args..., Value::Kind::V_LONG) {}

   protected:
   };
   
   
}

#endif
