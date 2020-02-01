
#ifndef __ASM_HPP
#error "include \"asm.hpp\", not \"asm-base.hpp\" directly"
#endif

#ifndef __ASM_BASE_HPP
#define __ASM_BASE_HPP

#include <list>
#include <vector>

#include "asm/asm-mem.hpp"
#include "asm/asm-reg.hpp"
#include "asm/asm-cond.hpp"
#include "util.hpp"

namespace zc { enum class Cond; }

namespace zc::z80 {

   class Value;
   class VariableValue;
   typedef std::vector<portal<const Value *>> Values;

   class Instruction;
   typedef std::list<Instruction *> Instructions;
   
   class Instruction {
   public:
      const Value *dst() const { return (operands().size() >= 1) ? *operands().front() : nullptr; }
      const Value *src() const {
         return (operands().size() >= 2) ? **++operands().begin() : nullptr;
      }
      const Values& operands() const { return operands_; }
      Values& operands() { return operands_; }
      const std::string& name() const { return name_; }
      FlagMod flagmod(const Flag& flag) const;

      virtual void Gen(std::list<const Value *>& vals) const {}
      virtual void Use(std::list<const Value *>& vals) const {}

      void ReplaceVar(const VariableValue *var, const Value *with);

      /**
       * Convert complex pseudo-instructions (e.g. ld hl,bc) to basic instructions 
       * (e.g. push bc \ pop bc)
       */
      void Resolve(Instructions& out);
                           
      virtual void Emit(std::ostream& os ) const;

      virtual bool Eq(const Instruction *other) const;
      virtual bool Match(const Instruction *to) const;
      
   protected:
      std::string name_;
      Values operands_;
      std::optional<Cond> cond_;

      virtual void Resolve_(Instructions& out) { out.push_back(this); }
      
      Instruction(const std::string& name): name_(name), operands_(), cond_(std::nullopt) {}
      Instruction(Cond cond, const std::string& name): name_(name), operands_(), cond_(cond) {}
      Instruction(const Values& operands, const std::string& name):
         name_(name), operands_(operands), cond_(std::nullopt) {}
      Instruction(const Values& operands, Cond cond, const std::string& name):
         name_(name), operands_(operands), cond_(cond) {}
   };

   class BinaryInstruction: public Instruction {
   public:
      virtual void Gen(std::list<const Value *>& vals) const override;
      virtual void Use(std::list<const Value *>& vals) const override;
      
   protected:
      template <typename... Args>
      BinaryInstruction(portal<const Value *> dst, portal<const Value *> src, Args... args):
         Instruction(Values {dst, src}, args...) {}
   };

   class UnaryInstruction: public Instruction {
   public:
      virtual void Gen(std::list<const Value *>& vals) const override;
      virtual void Use(std::list<const Value *>& vals) const override;
      
   protected:
      template <typename... Args>
      UnaryInstruction(const Value *dst, Args... args):
         Instruction(Values {dst}, args...) {}
   };

   class BitwiseInstruction: public BinaryInstruction {
   public:
      virtual void Gen(std::list<const Value *>& vals) const override;
      virtual void Use(std::list<const Value *>& vals) const override;

   protected:
      template <typename... Args>
      BitwiseInstruction(Args... args): BinaryInstruction(args...) {}
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
   class AndInstruction: public BitwiseInstruction {
   public:
      template <typename... Args>
      AndInstruction(Args... args): BitwiseInstruction(args..., "and") {}
   };

   /**
    * "CALL" instruction class
    */
   class CallInstruction: public UnaryInstruction {
   public:
      /* TODO -- this might need to be marked as using everything... */
      virtual void Gen(std::list<const Value *>& vals) const override;
      virtual void Use(std::list<const Value *>& vals) const override;
      
      template <typename... Args>
      CallInstruction(Args... args): UnaryInstruction(args..., "call") {}
   };

   /**
    * "CCF" instruction
    */
   class CcfInstruction: public Instruction {
   public:
      template <typename... Args>
      CcfInstruction(Args... args): Instruction(args..., "ccf") {}
   };   

   /**
    * "CP" instruction class
    */
   class CompInstruction: public BinaryInstruction {
   public:
      virtual void Gen(std::list<const Value *>& vals) const override {}

      template <typename... Args>
      CompInstruction(Args... args): BinaryInstruction(args..., "cp") {}
   };

   /**
    * "CPL" instruction
    */
   class CplInstruction: public Instruction {
   public:
      virtual void Gen(std::list<const Value *>& vals) const override;
      virtual void Use(std::list<const Value *>& vals) const override;
      
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
      virtual void Gen(std::list<const Value *>& vals) const override;
      virtual void Use(std::list<const Value *>& vals) const override;

      template <typename... Args>
      DjnzInstruction(Args... args): UnaryInstruction(args..., "djnz") {}
   };

   /**
    * "EX" instruction class
    */
   class ExInstruction: public BinaryInstruction {
   public:
      /* TODO -- Gen and Use are nuanced. */
      
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
      virtual void Gen(std::list<const Value *>& vals) const override {}
      virtual void Use(std::list<const Value *>& vals) const override {}
      
      template <typename... Args>
      JumpInstruction(Args... args): UnaryInstruction(args..., "jp") {}
   protected:
   };
   
   /**
    * "JR" instruction class
    */
   class JrInstruction: public UnaryInstruction {
   public:
      virtual void Emit(std::ostream& os) const override;
      
      template <typename... Args>
      JrInstruction(Args... args): UnaryInstruction(args..., "jr") {}
   };

   /**
    * "LD" instruction class
    */
   class LoadInstruction: public BinaryInstruction {
   public:
      virtual void Gen(std::list<const Value *>& vals) const override;
      virtual void Use(std::list<const Value *>& vals) const override;
      
      template <typename... Args>
      LoadInstruction(Args... args): BinaryInstruction(args..., "ld") {}

   protected:
      virtual void Resolve_(Instructions& out) override;      
   };

   /**
    * "LEA" instruction class
    */
   class LeaInstruction: public BinaryInstruction {
   public:
      virtual void Use(std::list<const Value *>& vals) const override {}
      
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
      virtual void Gen(std::list<const Value *>& vals) const override;
      virtual void Use(std::list<const Value *>& vals) const override;

      template <typename... Args>
      NegInstruction(Args... args): Instruction(args..., "neg") {}
   };

   /**
    * "OR" instruction class
    */
   class OrInstruction: public BitwiseInstruction {
   public:
      template <typename... Args>
      OrInstruction(Args... args): BitwiseInstruction(args..., "or") {}
   };

   /**
    * "PEA" instruction class
    */
   class PeaInstruction: public UnaryInstruction {
   public:
      virtual void Use(std::list<const Value *>& vals) const override {}
      
      template <typename... Args>
      PeaInstruction(Args... args): UnaryInstruction(args..., "pea") {}
   };

   /**
    * "POP" instruction class
    */
   class PopInstruction: public UnaryInstruction {
   public:
      virtual void Use(std::list<const Value *>& vals) const override {}
      
      template <typename... Args>
      PopInstruction(Args... args): UnaryInstruction(args..., "pop") {}
   };

   /**
    * "PUSH" instruction class
    */
   class PushInstruction: public UnaryInstruction {
   public:
      virtual void Gen(std::list<const Value *>& vals) const override {}

      template <typename... Args>
      PushInstruction(Args... args): UnaryInstruction(args..., "push") {}
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
      virtual void Gen(std::list<const Value *>& vals) const override;
      virtual void Use(std::list<const Value *>& vals) const override;
      
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
   class XorInstruction: public BitwiseInstruction {
   public:
      template <typename... Args>
      XorInstruction(Args... args): BitwiseInstruction(args..., "xor") {}
   };

   /*** PSEUDO-INSTRUCTIONS ***/
   
   class LabelInstruction: public Instruction {
   public:
      template <typename... Args>
      LabelInstruction(const Label *label): Instruction("%label"), label_(label) {}

      virtual void Emit(std::ostream& os) const override { label_->EmitDef(os); }
      virtual bool Eq(const Instruction *other) const override {
         auto other_ = dynamic_cast<const LabelInstruction *>(other);
         return other_ && label_->Eq(other_->label_);
      }
      virtual bool Match(const Instruction *to) const override { return Eq(to); }
      
   private:
      const Label *label_;
   };

}


#endif