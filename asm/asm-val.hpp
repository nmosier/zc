#ifndef __ASM_HPP
#error "include \"asm.hpp\""
#endif

#ifndef __ASM_VAL_HPP
#define __ASM_VAL_HPP

namespace zc::z80 {

   /**********
    * VALUES *
    **********/

   /**
    * Base class for values during code generation.
    */
   class Value {
   public:
      Size size() const { return size_; }

      Value(Size size): size_(size) {}
      
   protected:
      Size size_;
   };

   /**
    * Class representing immediate value.
    */
   class ImmediateValue: public Value {
   public:
      const intmax_t& imm() const { return imm_; }

      template <typename... Args>
      ImmediateValue(const intmax_t& imm, Args... args): imm_(imm), Value(args...) {}
      
   protected:
      intmax_t imm_;
   };

   /**
    * Class representing the value of a label (i.e. its address).
    */
   class LabelValue: public Value {
   public:
      const Label *label() const { return label_; }

      template <typename... Args>
      LabelValue(const Label *label, Args... args): Value(args...), label_(label) {}
      
   protected:
      const Label *label_;
   };

   /**
    * Class representing a value held in a single-byte register.
    */
   template <class RegType>
   class RegisterValue: public Value {
   public:
      const RegType *reg() const override { return reg(); }

      template <typename... Args>
      RegisterValue(const RegType *reg, Args... args): Value(args...), reg_(reg) {}
      
   protected:
      const RegType *reg_;
   };

   class ByteRegister;
   class MultibyteRegister;
   typedef RegisterValue<ByteRegister> ByteRegValue;
   typedef RegisterValue<MultibyteRegister> MultibyteRegValue;


   /**
    * Class representing an indexed register.
    */
   class IndexedValue: public Value {
   public:
      const MultibyteRegValue *val() const { return val_; }
      int8_t index() const { return index_; }

      template <typename... Args>
      IndexedValue(const MultibyteRegValue *val, int8_t index, Args... args):
         Value(args...), val_(val), index_(index) {}
      
   protected:
      const MultibyteRegValue *val_;
      int8_t index_;
   };

   /**
    * Class representing a value contained in memory.
    */
   class MemoryValue: public Value {
   public:
      const MemoryLocation *loc() const { return loc_; }

      template <typename... Args>
      MemoryValue(const MemoryLocation *loc, Args... args): Value(args...), loc_(loc) {}
      
   protected:
      const MemoryLocation *loc_;
      
   };
   
}

#endif
