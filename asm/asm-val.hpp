#ifndef __ASM_HPP
#error "include \"asm.hpp\""
#endif

#ifndef __ASM_VAL_HPP
#define __ASM_VAL_HPP

#include <ostream>

namespace zc::z80 {

   /**********
    * VALUES *
    **********/

   /**
    * Base class for values during code generation.
    */
   template <Size sz>
   class Value {
   public:
      virtual void Emit(std::ostream& os) const = 0;
      
   protected:
   };

   /**
    * Class representing immediate value.
    */
   template <Size sz>
   class ImmediateValue: public Value<sz> {
   public:
      const intmax_t& imm() const { return imm_; }

      virtual void Emit(std::ostream& os) const override;

      template <typename... Args>
      ImmediateValue(const intmax_t& imm, Args... args): imm_(imm), Value<sz>(args...) {}
      
   protected:
      intmax_t imm_;
   };

   /**
    * Class representing the value of a label (i.e. its address).
    */
   class LabelValue: public Value<Size::LONG> {
   public:
      const Label *label() const { return label_; }

      virtual void Emit(std::ostream& os) const override;

      template <typename... Args>
      LabelValue(const Label *label, Args... args): Value(args...), label_(label) {}
      
   protected:
      const Label *label_;
   };

   template <Size sz> class Register;
   /**
    * Class representing a value held in a single-byte register.
    */
   template <Size sz>
   class RegisterValue: public Value<sz> {
   public:
      const Register<sz> *reg() const override { return reg(); }

      virtual void Emit(std::ostream& os) const override;

      template <typename... Args>
      RegisterValue(const Register<sz> *reg, Args... args): Value<sz>(args...), reg_(reg) {}
      
   protected:
      const Register<sz> *reg_;
   };

   /**
    * Class representing an indexed register value.
    */
   template <Size sz>
   class IndexedRegisterValue: public Value<sz> {
   public:
      const RegisterValue<Size::LONG> *val() const { return val_; }
      int8_t index() const { return index_; }
      
      virtual void Emit(std::ostream& os) const override;
      
      template <typename... Args>
      IndexedRegisterValue(const RegisterValue<Size::LONG> *val,
                           const ImmediateValue<Size::BYTE> *index,
                           Args... args):
         Value<sz>(args...), val_(val), index_(index) {}
      
   protected:
      const RegisterValue<Size::LONG> *val_;
      const ImmediateValue<Size::BYTE> *index_; 
   };

   /**
    * Class representing a value with a fixed added offset.
    */
   template <Size sz>
   class OffsetValue: public Value<sz> {
   public:
      const Value<sz> *base() const { return base_; }
      const intmax_t& offset() const { return offset_; }

      virtual void Emit(std::ostream& os) const override;

      template <typename... Args>
      OffsetValue(const Value<sz> *base, const intmax_t& offset, Args... args):
         Value<sz>(args...), base_(base), offset_(offset) {}

   protected:
      const Value<sz> base_;
      const intmax_t offset_;
   };

   /**
    * Class representing a value contained in memory.
    */
   template <Size sz>
   class MemoryValue: public Value<sz> {
   public:
      const MemoryLocation<sz> *loc() const { return loc_; }

      virtual void Emit(std::ostream& os) const override;

      template <typename... Args>
      MemoryValue(const MemoryLocation<sz> *loc, Args... args): Value<sz>(args...), loc_(loc) {}
      
   protected:
      const MemoryLocation<sz> *loc_;
   };
   
}

#endif
