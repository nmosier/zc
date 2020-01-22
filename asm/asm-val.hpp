#ifndef __ASM_HPP
#error "include \"asm.hpp\""
#endif

#ifndef __ASM_VAL_HPP
#define __ASM_VAL_HPP

#include <ostream>

#include "asm-reg.hpp"
#include "util.hpp"

namespace zc::z80 {

   /**********
    * VALUES *
    **********/

   class VariableValue;
   
   /**
    * Base class for values during code generation.
    */
   class Value {
   public:
      int size() const { return size_.get(); }
      virtual const Register *reg() const { return nullptr; }
      virtual const VariableValue *var() const { return nullptr; }
      virtual const Value **varptr(const Value **hint) const { return nullptr; }
      
      virtual void Emit(std::ostream& os) const = 0;
      virtual Value *Add(const intmax_t& offset) const = 0;
      bool Eq(const Value *other) const {
         return size() == other->size() && Eq_(other);
      }
      bool Match(const Value *to) const;
      
   protected:
      portal<int> size_;

      virtual bool Eq_(const Value *other) const = 0;
      virtual bool Match_(const Value *to) const = 0;
      
      Value(portal<int> size): size_(size) {}
   };

   template <class Derived>
   class Value_: public Value {
   protected:
      virtual bool Eq_(const Value *other) const override {
         auto derived = dynamic_cast<const Derived *>(other);
         return derived ? Eq_aux(derived) : false;
      }

      virtual bool Match_(const Value *to) const override {
         auto derived = dynamic_cast<const Derived *>(to);
         return derived ? Match_aux(derived) : false;
      }

      virtual bool Eq_aux(const Derived *other) const = 0;
      virtual bool Match_aux(const Derived *to) const = 0;
      
      template <typename... Args>
      Value_(Args... args): Value(args...) {}
   };

   /**
    * Variable that hasn't been assigned a storage class.
    */
   class VariableValue: public Value_<VariableValue> {
   public:
      int id() const { return id_; }
      virtual const VariableValue *var() const override { return this; }
      virtual const Value **varptr(const Value **hint) const override { return hint; }
      virtual void Emit(std::ostream& os) const override;
      virtual Value *Add(const intmax_t& offset) const override
      { throw std::logic_error("attempted to add to abstract value"); }

      VariableValue(int size): Value_(size), id_(id_counter_++) {}
      
   protected:
      int id_;
      static int id_counter_;

      virtual bool Eq_aux(const VariableValue *other) const override { return id() == other->id(); }
      virtual bool Match_aux(const VariableValue *to) const override { return Eq_aux(to); }
   };

   /**
    * Class representing immediate value.
    */
   class ImmediateValue: public Value_<ImmediateValue> {
   public:
      const intmax_t& imm() const { return *imm_; }
      
      virtual void Emit(std::ostream& os) const override;
      virtual Value *Add(const intmax_t& offset) const override;
      
      ImmediateValue(portal<intmax_t> imm, portal<int> size): Value_(size), imm_(imm) {}
      ImmediateValue(const intmax_t& imm); /* infers size */
      
   protected:
      portal<intmax_t> imm_;
      
      virtual bool Eq_aux(const ImmediateValue *other) const override {
         return imm() == other->imm();
      }
      virtual bool Match_aux(const ImmediateValue *to) const override;
   };

   /**
    * Class representing the value of a label (i.e. its address).
    */
   class LabelValue: public Value_<LabelValue> {
   public:
      const Label *label() const { return label_; }

      virtual void Emit(std::ostream& os) const override;
      virtual Value *Add(const intmax_t& offset) const override;

      template <typename... Args>
      LabelValue(const Label *label, Args... args): Value_(args..., long_size), label_(label) {}
      
   protected:
      const Label *label_;

      virtual bool Eq_aux(const LabelValue *other) const override {
         return label()->Eq(other->label());
      }
      virtual bool Match_aux(const LabelValue *to) const override { return Eq_aux(to); }
   };

   class Register;
   /**
    * Class representing a value held in a single-byte register.
    */
   class RegisterValue: public Value_<RegisterValue> {
   public:
      virtual const Register *reg() const override { return *reg_; }
      
      virtual void Emit(std::ostream& os) const override;
      virtual Value *Add(const intmax_t& offset) const override;

      RegisterValue(const ByteRegister *reg): Value_(byte_size), reg_(reg) {}
      RegisterValue(const MultibyteRegister *reg): Value_(long_size), reg_(reg) {}
      RegisterValue(const Register *reg): Value_(reg->size()), reg_(reg) {}            
      RegisterValue(const Register *reg, int size): Value_(size), reg_(reg) {}
      RegisterValue(const Register **reg_ptr, portal<int> size): Value_(size), reg_(reg_ptr) {}
      
   protected:
      portal<const Register *> reg_;

      virtual bool Eq_aux(const RegisterValue *other) const override {
         return reg()->Eq(other->reg());
      }
      virtual bool Match_aux(const RegisterValue *to) const override;
   };
   
   /**
    * Class representing an indexed register value.
    */
   class IndexedRegisterValue: public Value_<IndexedRegisterValue> {
   public:
      const RegisterValue *val() const { return *val_; }
      int8_t index() const { return *index_; }
      virtual const Register *reg() const override { return val()->reg(); }
      
      virtual void Emit(std::ostream& os) const override;
      virtual Value *Add(const intmax_t& offset) const override;

      IndexedRegisterValue(const RegisterValue *val, const portal<int8_t>& index):
         Value_(long_size), val_(val), index_(index) {}
      IndexedRegisterValue(const RegisterValue **val, const portal<int8_t>& index,
                           const portal<int>& size):
         Value_(size), val_(val), index_(index) {}

      
   protected:
      portal<const RegisterValue *> val_;
      portal<int8_t> index_;

      virtual bool Eq_aux(const IndexedRegisterValue *other) const override {
         return val()->Eq(other->val()) && index() == other->index();
      }
      virtual bool Match_aux(const IndexedRegisterValue *to) const override;
   };

   /**
    * Class representing a value with a fixed added offset.
    */
   class OffsetValue: public Value_<OffsetValue> {
   public:
      const Value *base() const { return base_; }
      const intmax_t& offset() const { return offset_; }
      virtual const Register *reg() const override { return base()->reg(); }

      virtual void Emit(std::ostream& os) const override;
      virtual Value *Add(const intmax_t& offset) const override;

      template <typename... Args>
      OffsetValue(const Value *base, const intmax_t& offset, Args... args):
         Value_(args...), base_(base), offset_(offset) {}

   protected:
      const Value *base_;
      const intmax_t offset_;

      virtual bool Eq_aux(const OffsetValue *other) const override {
         return base()->Eq(other->base()) && offset() == other->offset();
      }
      virtual bool Match_aux(const OffsetValue *to) const override { return Eq_aux(to); }
   };

   /**
    * Class representing a value contained in memory.
    */
   class MemoryValue: public Value_<MemoryValue> {
   public:
      const MemoryLocation *loc() const { return *loc_; }
      virtual const Register *reg() const override {return loc()->addr()->reg(); }
      virtual const VariableValue *var() const override { return loc()->addr()->var(); }
      //virtual const Value **varptr(const Value **hint) const override { return &loc()->addr_; }
      /* TODO */
      
      virtual void Emit(std::ostream& os) const override;
      virtual Value *Add(const intmax_t& offset) const override;

      MemoryValue *Next(int size) const;
      MemoryValue *Prev(int size) const;
      
      MemoryValue(portal<const MemoryLocation *> loc, portal<int> size):
         Value_(size), loc_(loc) {}
      
   protected:
      portal<const MemoryLocation *> loc_;

      virtual bool Eq_aux(const MemoryValue *other) const override {
         return loc()->Eq(other->loc());
      }
      virtual bool Match_aux(const MemoryValue *to) const override;
   };
   
   /*** EXTERNAL CONSTANTS ***/
   template <intmax_t N>
   const ImmediateValue imm_b(N, byte_size);

   template <intmax_t N>
   const ImmediateValue imm_l(N, long_size);
   
}

#endif
