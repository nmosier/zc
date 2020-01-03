
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

   class BinaryInstruction: public Instruction {
   public:
      const Value *dst() const { return dst_; }
      const Value *src() const { return src_; }

      virtual void Emit(std::ostream& os) const override;

      template <typename... Args>
      BinaryInstruction(const Value *dst, const Value *src, Args... args):
         Instruction(args...), dst_(dst), src_(src) {}
      
   protected:
      const Value *dst_, *src_; 
   };

   class UnaryInstruction: public Instruction {
   public:
      const Value *dst() const { return dst_; }

      virtual void Emit(std::ostream& os) const override;

      template <typename... Args>
      UnaryInstruction(const Value *dst, Args... args): Instruction(args...), dst_(dst) {}
      
   protected:
      const Value *dst_;
   };

   /***********************
    * INSTRUCTION CLASSES *
    ***********************/

   /**
    * "ADC" instruction class.
    */
   class AdcInstruction: public BinaryInstruction {
   public:
      template <typename... Args>
      AdcInstruction(Args... args): BinaryInstruction(args..., "adc") {}
   };

   
   /**
    * "ADD" instruction class.
    */
   class AddInstruction: public BinaryInstruction {
   public:
      template <typename... Args>
      AddInstruction(Args... args): BinaryInstruction(args..., "add") {}
   };

   /**
    * "AND" instruction class.
    */
   class AndInstruction: public BinaryInstruction {
   public:
      template <typename... Args>
      AndInstruction(Args... args): BinaryInstruction(args..., "and") {}
   };

   /**
    * "CALL" instruction class
    */
   class CallInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      CallInstruction(Args... args): UnaryInstruction(args..., "call") {}
   };

   /**
    * "CP" instruction class
    */
   class CompInstruction: public BinaryInstruction {
   public:
      template <typename... Args>
      CompInstruction(Args... args): BinaryInstruction(args..., "cp") {}
   };

   /**
    * "CPL" instruction
    */
   class CplInstruction: public Instruction {
   public:
      template <typename... Args>
      CplInstruction(Args... args): Instruction(args..., "cpl") {}
   };

   /**
    * "DEC" instruction class
    */
   class DecInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      DecInstruction(Args... args): UnaryInstruction(args..., "dec") {}
   };

   /**
    * "DJNZ" instruction
    */
   class DjnzInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      DjnzInstruction(Args... args): UnaryInstruction(args..., "djnz") {}
   };

   /**
    * "EX" instruction class
    */
   class ExInstruction: public BinaryInstruction {
   public:
      template <typename... Args>
      ExInstruction(Args... args): BinaryInstruction(args..., "ex") {}
   };

   /**
    * "INC" instruction class
    */
   class IncInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      IncInstruction(Args... args): UnaryInstruction(args..., "inc") {}
   };

   /**
    * "JP" instruction class
    */
   class JumpInstruction: public UnaryInstruction {
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
   class LoadInstruction: public BinaryInstruction {
   public:
      template <typename... Args>
      LoadInstruction(Args... args): BinaryInstruction(args..., "ld") {}
   };

   /**
    * "LEA" instruction class
    */
   class LeaInstruction: public BinaryInstruction {
   public:
      template <typename... Args>
      LeaInstruction(Args... args): BinaryInstruction(args..., "lea") {}
   };

   /**
    * "MLT" instruction class
    */
   class MultInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      MultInstruction(Args... args): UnaryInstruction(args..., "mlt") {}
   };

   /**
    * "NEG" instruction class
    */
   class NegInstruction: public Instruction {
   public:
      template <typename... Args>
      NegInstruction(Args... args): Instruction(args..., "neg") {}
   };

   /**
    * "OR" instruction class
    */
   class OrInstruction: public BinaryInstruction {
   public:
      template <typename... Args>
      OrInstruction(Args... args): BinaryInstruction(args..., "or") {}
   };

   /**
    * "PEA" instruction class
    */
   class PeaInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      PeaInstruction(Args... args): UnaryInstruction(args..., "pea") {}
   };

   /**
    * "POP" instruction class
    */
   class PopInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      PopInstruction(Args... args): UnaryInstruction(args..., "pop") {}
   };

   /**
    * "PUSH" instruction class
    */
   class PushInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      PushInstruction(Args... args): UnaryInstruction(args..., "pop") {}
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
   class RlInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      RlInstruction(Args... args): UnaryInstruction(args..., "rl") {}
   };

   /**
    * "RLC" instruction class
    */
   class RlcInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      RlcInstruction(Args... args): UnaryInstruction(args..., "rlc") {}
   };

   /**
    * "RR" instruction class
    */
   class RrInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      RrInstruction(Args... args): UnaryInstruction(args..., "rr") {}
   };

   /**
    * "RRC" instruction class
    */
   class RrcInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      RrcInstruction(Args... args): UnaryInstruction(args..., "rrc") {}
   };

   /**
    * "SBC" instruction class
    */
   class SbcInstruction: public BinaryInstruction {
   public:
      template <typename... Args>
      SbcInstruction(Args... args): BinaryInstruction(args..., "sbc") {}
   };

   /**
    * "SCF" instruction
    */
   class ScfInstruction: public Instruction {
   public:
      template <typename... Args>
      ScfInstruction(Args... args): Instruction(args..., "scf") {}
   };

   /**
    * "SLA" instruction class
    */
   class SlaInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      SlaInstruction(Args... args): UnaryInstruction(args..., "sla") {}
   };

   /**
    * "SRA" instruction class
    */
   class SraInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      SraInstruction(Args... args): UnaryInstruction(args..., "sra") {}
   };

   /**
    * "SRL" instruction class
    */
   class SrlInstruction: public UnaryInstruction {
   public:
      template <typename... Args>
      SrlInstruction(Args... args): UnaryInstruction(args..., "srl") {}
   };
   
   /**
    * "SUB" instruction class
    */
   class SubInstruction: public BinaryInstruction {
   public:
      template <typename... Args>
      SubInstruction(Args... args): BinaryInstruction(args..., "sub") {}
   };
   
   /**
    * "XOR" instruction class
    */
   class XorInstruction: public BinaryInstruction {
   public:
      template <typename... Args>
      XorInstruction(Args... args): BinaryInstruction(args..., "xor") {}
   };

   
}


#endif
