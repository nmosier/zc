
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

      /**
       * Name of opcode
       */
      const char *name() const { return name_; }

      /**
       * Dump assembly for instruction. 
       */
      virtual void Emit(std::ostream& os) const;
      
      /** 
       * Number of cycles required to execute the instruction.
       */
      // virtual int cycles() const = 0;

      Instruction(const char *name): name_(name) {}

   protected:
      const char *name_;
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
      template <typename... Args>
      AddInstruction(Args... args): BinaryInstruction<sz>(args..., "add") {}
   };

   /**
    * "AND" instruction class.
    */
   class AndInstruction: public BinaryInstruction<Size::BYTE> {
   public:
      template <typename... Args>
      AndInstruction(Args... args): BinaryInstruction(args..., "and") {}
   };

   /**
    * "CALL" instruction class
    */
   class CallInstruction: public UnaryInstruction<Size::LONG> {
   public:
      template <typename... Args>
      CallInstruction(Args... args): UnaryInstruction<Size::LONG>(args..., "call") {}
   };

   /**
    * "CP" instruction class
    */
   class CompInstruction: public BinaryInstruction<Size::BYTE> {
   public:
      template <typename... Args>
      CompInstruction(Args... args): UnaryInstruction<Size::LONG>(args..., "cp") {}
   };

   /**
    * "DEC" instruction class
    */
   template <Size sz>
   class DecInstruction: public UnaryInstruction<sz> {
   public:
      template <typename... Args>
      DecInstruction(Args... args): UnaryInstruction<sz>(args..., "dec") {}
   };

   /**
    * "DJNZ" instruction
    */
   class DjnzInstruction: public UnaryInstruction<Size::BYTE> {
   public:
      template <typename... Args>
      DjnzInstruction(Args... args): UnaryInstruction<Size::BYTE>(args..., "djnz") {}
   };

   /**
    * "EX" instruction class
    */
   class ExInstruction: public BinaryInstruction<Size::LONG> {
   public:
      template <typename... Args>
      ExInstruction(Args... args): BinaryInstruction<Size::LONG>(args..., "ex") {}
   };

   /**
    * "INC" instruction class
    */
   class IncInstruction: public UnaryInstruction<Size::BYTE> {
   public:
      template <typename... Args>
      IncInstruction(Args... args): UnaryInstruction<Size::BYTE>(args..., "inc") {}
   };

   /**
    * "JP" instruction class
    */
   class JumpInstruction: public UnaryInstruction<Size::LONG> {
   public:
      template <typename... Args>
      JumpInstruction(Args... args): UnaryInstruction(args..., "jp") {}
   protected:
   };

   /**
    * "JP cc,Mmm" instruction class
    */
   class JumpCondInstruction: public JumpInstruction {
   public:
      const FlagState *cond() const { return cond_; }
      
      virtual void Emit(std::ostream& os) const override;

      template <typename... Args>
      JumpCondInstruction(const FlagState *cond, bool state, Args... args):
         JumpInstruction(args...), cond_(cond) {}
      
   protected:
      const FlagState *cond_;
   };

   /**
    * "LD" instruction class
    */
   template <Size sz>
   class LoadInstruction: public BinaryInstruction<sz> {
   public:
      template <typename... Args>
      LoadInstruction(Args... args): BinaryInstruction<sz>(args..., "ld") {}
   };

   /**
    * "LEA" instruction class
    */
   class LeaInstruction: public BinaryInstruction<Size::LONG> {
   public:
      template <typename... Args>
      LeaInstruction(Args... args): BinaryInstruction<Size::LONG>(args..., "lea") {}
   };

   /**
    * "OR" instruction class
    */
   class OrInstruction: public BinaryInstruction<Size::BYTE> {
   public:
      template <typename... Args>
      OrInstruction(Args... args): BinaryInstruction<Size::BYTE>(args..., "or") {}
   };

   /**
    * "PEA" instruction class
    */
   class PeaInstruction: public UnaryInstruction<Size::LONG> {
   public:
      template <typename... Args>
      PeaInstruction(Args... args): UnaryInstruction<Size::LONG>(args..., "pea") {}
   };

   /**
    * "POP" instruction class
    */
   class PopInstruction: public UnaryInstruction<Size::LONG> {
   public:
      template <typename... Args>
      PopInstruction(Args... args): UnaryInstruction<Size::LONG>(args..., "pop") {}
   };

   /**
    * "PUSH" instruction class
    */
   class PushInstruction: public UnaryInstruction<Size::LONG> {
   public:
      template <typename... Args>
      PushInstruction(Args... args): UnaryInstruction<Size::LONG>(args..., "pop") {}
   };

   /**
    * "RET" instruction class
    */
   class RetInstruction: public Instruction {
   public:
      template <typename... Args>
      RetInstruction(Args... args): Instruction(args..., "ret") {}
   };

   /**
    * "RET cc" instruction class
    */
   class RetCondInstruction: public RetInstruction {
   public:
      const FlagState *cond() const { return cond_; }

      virtual void Emit(std::ostream& os) const override;
      
      template <typename... Args>
      RetCondInstruction(const FlagState *cond, Args... args):
         RetInstruction(args...), cond_(cond) {}
      
   protected:
      const FlagState *cond_;
   };
   
   /**
    * "RL" instruction class
    */
   class RlInstruction: public UnaryInstruction<Size::BYTE> {
   public:
      template <typename... Args>
      RlInstruction(Args... args): UnaryInstruction<Size::BYTE>(args..., "rl") {}
   };

   /**
    * "RLC" instruction class
    */
   class RlcInstruction: public UnaryInstruction<Size::BYTE> {
   public:
      template <typename... Args>
      RlcInstruction(Args... args): UnaryInstruction<Size::BYTE>(args..., "rlc") {}
   };

   /**
    * "RR" instruction class
    */
   class RrInstruction: public UnaryInstruction<Size::BYTE> {
   public:
      template <typename... Args>
      RrInstruction(Args... args): UnaryInstruction<Size::BYTE>(args..., "rr") {}
   };

   /**
    * "RRC" instruction class
    */
   class RrcInstruction: public UnaryInstruction<Size::BYTE> {
   public:
      template <typename... Args>
      RrcInstruction(Args... args): UnaryInstruction<Size::BYTE>(args..., "rrc") {}
   };

   /**
    * "SBC" instruction class
    */
   template <Size sz>
   class SbcInstruction: public BinaryInstruction<sz> {
   public:
      template <typename... Args>
      SbcInstruction(Args... args): BinaryInstruction<sz>(args..., "sbc") {}
   };

   /**
    * "SLA" instruction class
    */
   class SlaInstruction: public UnaryInstruction<Size::BYTE> {
   public:
      template <typename... Args>
      SlaInstruction(Args... args): UnaryInstruction<Size::BYTE>(args..., "sla") {}
   };

   /**
    * "SRA" instruction class
    */
   class SraInstruction: public UnaryInstruction<Size::BYTE> {
   public:
      template <typename... Args>
      SraInstruction(Args... args): UnaryInstruction<Size::BYTE>(args..., "sra") {}
   };

   /**
    * "SRL" instruction class
    */
   class SrlInstruction: public UnaryInstruction<Size::BYTE> {
   public:
      template <typename... Args>
      SrlInstruction(Args... args): UnaryInstruction<Size::BYTE>(args..., "srl") {}
   };
   
   /**
    * "SUB" instruction class
    */
   class SubInstruction: public BinaryInstruction<Size::BYTE> {
   public:
      template <typename... Args>
      SubInstruction(Args... args): BinaryInstruction<Size::BYTE>(args..., "sub") {}
   };
   
   /**
    * "XOR" instruction class
    */
   class XorInstruction: public UnaryInstruction<Size::BYTE> {
   public:
      template <typename... Args>
      XorInstruction(Args... args): UnaryInstruction<Size::BYTE>(args..., "xor") {}
   };

}


#endif
