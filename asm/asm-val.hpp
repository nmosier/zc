#ifndef __ASM_HPP
#error "include \"asm.hpp\""
#endif

#ifndef __ASM_VAL_HPP
#define __ASM_VAL_HPP

#include <ostream>

#include "asm-reg.hpp"

namespace zc::z80 {

   /**********
    * VALUES *
    **********/

   /**
    * Base class for values during code generation.
    */
   class Value {
   public:
      int size() const { return size_; }
      
      virtual void Emit(std::ostream& os) const = 0;
      virtual Value *Add(const intmax_t& offset) const = 0;
      
   protected:
      int size_;

      Value(int size): size_(size) {}
   };

   /**
    * Class representing immediate value.
    */
   class ImmediateValue: public Value {
   public:
      const intmax_t& imm() const { return imm_; }
      
      virtual void Emit(std::ostream& os) const override;
      virtual Value *Add(const intmax_t& offset) const override;
      
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

      virtual void Emit(std::ostream& os) const override;
      virtual Value *Add(const intmax_t& offset) const override;      

      template <typename... Args>
      LabelValue(const Label *label, Args... args): Value(args..., long_size), label_(label) {}
      
   protected:
      const Label *label_;
   };

   class Register;
   /**
    * Class representing a value held in a single-byte register.
    */
   class RegisterValue: public Value {
   public:
      const Register *reg() const { return reg(); }
      
      virtual void Emit(std::ostream& os) const override;
      virtual Value *Add(const intmax_t& offset) const override;

      template <typename... Args>
      RegisterValue(const Register *reg, Args... args):
         Value(args..., reg->size()), reg_(reg) {}
      
   protected:
      const Register *reg_;
   };
   
   /**
    * Class representing an indexed register value.
    */
   class IndexedRegisterValue: public Value {
   public:
      const RegisterValue *val() const { return val_; }
      int8_t index() const { return index_; }
      
      virtual void Emit(std::ostream& os) const override;
      virtual Value *Add(const intmax_t& offset) const override;
      
      template <typename... Args>
      IndexedRegisterValue(const RegisterValue *val,
                           int8_t index,
                           Args... args):
         Value(args...), val_(val), index_(index) {}
      
   protected:
      const RegisterValue *val_;
      int8_t index_;
   };

   /**
    * Class representing a value with a fixed added offset.
    */
   class OffsetValue: public Value {
   public:
      const Value *base() const { return base_; }
      const intmax_t& offset() const { return offset_; }

      virtual void Emit(std::ostream& os) const override;
      virtual Value *Add(const intmax_t& offset) const override;

      template <typename... Args>
      OffsetValue(const Value *base, const intmax_t& offset, Args... args):
         Value(args...), base_(base), offset_(offset) {}

   protected:
      const Value *base_;
      const intmax_t offset_;
   };

   /**
    * Class representing a value contained in memory.
    */
   class MemoryValue: public Value {
   public:
      const MemoryLocation *loc() const { return loc_; }
      
      virtual void Emit(std::ostream& os) const override;
      virtual Value *Add(const intmax_t& offset) const override;

      MemoryValue *Next(int size) const;
      MemoryValue *Prev(int size) const;
      
      template <typename... Args>
      MemoryValue(const MemoryLocation *loc, Args... args): Value(args...), loc_(loc) {}
      
   protected:
      const MemoryLocation *loc_;
   };


   /*** EXTERNAL CONSTANTS ***/
   template <intmax_t N>
   const ImmediateValue imm_b(N, byte_size);

   template <intmax_t N>
   const ImmediateValue imm_l(N, long_size);
   
}

#endif
