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
      enum class Kind {V_BYTE, V_WORD, V_LONG};
      Kind kind() const { return kind_; }
      int size() const;

      static int size(Kind kind);
      
      Value(Kind kind): kind_(kind) {}
      
   protected:
      Kind kind_;
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

   class Register;
   /**
    * Class representing a value contained in a register.
    */
   class RegisterValue: public Value {
   public:
      const Register *reg() const { return reg_; }

      template <typename... Args>
      RegisterValue(const Register *reg, Args... args): Value(args...), reg_(reg) {}
      
   protected:
      const Register *reg_;
   };

   /**
    * Class represneting a value contained in memory.
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
