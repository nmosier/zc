
#ifndef __ASM_HPP
#error "include \"asm.hpp\", not \"asm-base.hpp\" directly"
#endif

#ifndef __ASM_BASE_HPP
#define __ASM_BASE_HPP

#include "asm/asm-mem.hpp"
#include "asm/asm-reg.hpp"
#include "asm/asm-cond.hpp"

namespace zc::z80 {

   class Instruction {
   public:

      virtual const char *name() const = 0;

      /**
       * Dump assembly for instruction. 
       */
      virtual void Emit(std::ostream& os) const = 0;
      
      /** 
       * Number of cycles required to execute the instruction.
       */
      // virtual int cycles() const = 0;

   protected:
   };


   template <Size sz>
   class BinaryInstruction: public Instruction {
   public:
      const Value<sz> *dst() const { return dst_; }
      const Value<sz> *src() const { return src_; }

      virtual void Emit(std::ostream& os) const override;

      template <typename... Args>
      BinaryInstruction(const Value<sz> *dst, const Value<sz> *src, Args... args):
         Instruction(args...), dst_(dst), src_(src) {}
      
   protected:
      const Value<sz> *dst_, *src_; 
   };

   template <Size sz>
   class UnaryInstruction: public Instruction {
   public:
      const Value<sz> *dst() const { return dst_; }

      virtual void Emit(std::ostream& os) const override;

      template <typename... Args>
      UnaryInstruction(const Value<sz> *dst, Args... args): Instruction(args...), dst_(dst) {}
      
   protected:
      const Value<sz> *dst_;
   };

   /***********************
    * INSTRUCTION CLASSES *
    ***********************/

   /**
    * "ADD" instruction class.
    */
   template <Size sz>
   class AddInstruction: public BinaryInstruction<sz> {
   public:
      virtual const char *name() const override { return "add"; }
      
      template <typename... Args>
      AddInstruction(Args... args): BinaryInstruction<sz>(args...) {}
      
   protected:
   };

   /**
    * "AND" instruction class.
    */
   class AndInstruction: public BinaryInstruction<Size::BYTE> {
   public:
      virtual const char *name() const override { return "and"; }

      template <typename... Args>
      AndInstruction(Args... args): BinaryInstruction(args...) {}
   };

   /**
    * "CALL" instruction class
    */
   class CallInstruction: public UnaryInstruction<Size::LONG> {
   public:
      virtual const char *name() const override { return "call"; }

      template <typename... Args>
      CallInstruction(Args... args): UnaryInstruction<Size::LONG>(args...) {}
      
   protected:
   };

   /**
    * "CP" instruction class
    */
   class CompInstruction: public BinaryInstruction<Size::BYTE> {
   public:
      virtual const char *name() const override { return "cp"; }

      template <typename... Args>
      CompInstruction(Args... args): UnaryInstruction<Size::LONG>(args...) {}
   };

   /**
    * "DEC" instruction class
    */
   template <Size sz>
   class DecInstruction: public UnaryInstruction<sz> {
   public:
      virtual const char *name() const override { return "dec"; }

      template <typename... Args>
      DecInstruction(Args... args): UnaryInstruction<sz>(args...) {}
   };

   /**
    * "DJNZ" instruction
    */
   class DjnzInstruction: public UnaryInstruction<Size::BYTE> {
   public:
      virtual const char *name() const override { return "djnz"; }

      template <typename... Args>
      DjnzInstruction(Args... args): UnaryInstruction<Size::BYTE>(args...) {}
      
   protected:
   };

   /**
    * "EX" instruction class
    */
   class ExInstruction: public BinaryInstruction<Size::LONG> {
   public:
      virtual const char *name() const override { return "ex"; }

      template <typename... Args>
      ExInstruction(Args... args): BinaryInstruction<Size::LONG>(args...) {}
      
   protected:
   };

   /**
    * "INC" instruction class
    */
   class IncInstruction: public UnaryInstruction<Size::BYTE> {
   public:
      virtual const char *name() const override { return "inc"; }

      template <typename... Args>
      IncInstruction(Args... args): UnaryInstruction<Size::BYTE>(args...) {}
   };

   /**
    * "JP" instruction class
    */
   class JumpInstruction: public UnaryInstruction<Size::LONG> {
   public:
      virtual const char *name() const override { return "jp"; }

      template <typename... Args>
      JumpInstruction(Args... args): UnaryInstruction(args...) {}
   protected:
   };

   /**
    * "JP cc,Mmm" instruction class
    */
   class JumpCondInstruction: public JumpInstruction {
   public:
      const Flag *flag() const { return flag_; }
      bool state() const { return state_; }
      
      virtual void Emit(std::ostream& os) const override;

      template <typename... Args>
      JumpCondInstruction(const Flag *flag, bool state, Args... args):
         JumpInstruction(args...), flag_(flag) {}
      
   protected:
      const Flag *flag_;
      bool state_;
   };

   /**
    * "LD" instruction class
    */
   template <Size sz>
   class LoadInstruction: public BinaryInstruction<sz> {
   public:
      virtual const char *name() const override { return "ld"; }

      template <typename... Args>
      LoadInstruction(Args... args): BinaryInstruction<sz>(args...) {}
   };

   /**
    * "OR" instruction class
    */
   class OrInstruction: public BinaryInstruction<Size::BYTE> {
   public:
      virtual const char *name() const override { return "or"; }

      template <typename... Args>
      OrInstruction(Args... args): BinaryInstruction<Size::BYTE>(args...) {}
   };
   
}


#endif
