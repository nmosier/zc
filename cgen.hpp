#ifndef __CGEN_HPP
#define __CGEN_HPP

#include "ast.hpp"
#include "asm.hpp"
#include "scopedtab.hpp"

namespace zc {

   using namespace z80;

   class SymInfo {
   public:
      virtual const MemoryLocation *loc() const = 0;
      const ASTType *type() const { return type_; }

   protected:
      const ASTType *type_;

      SymInfo(const ASTType *type): type_(type) {}
   };

   class FunctionImpl;
   class FunctionInfo: public SymInfo {
   public:
      typedef std::vector<const MemoryValue> ArgVec;
      
      const FunctionImpl *impl() const { return impl_; }
      virtual const MemoryLocation *loc() const override;

      template <typename... Args>
      FunctionInfo(const FunctionImpl *impl, Args... args): SymInfo(args...), impl_(impl) {}
      
   protected:
      const FunctionImpl *impl_;
      const MemoryValue *next_local_;
      ArgVec argvec_;
   };

   class GlobalVarInfo: public SymInfo {
   public:
      const MemoryValue *val() const { return val_; }
      virtual const MemoryLocation *loc() const override { return val()->loc(); }

      template <typename... Args>
      GlobalVarInfo(const MemoryValue *val, Args... args): SymInfo(args...), val_(val) {}
      
   protected:
      const MemoryValue *val_;
   };

   enum class Cond: int
      {Z = 0x1,
       NZ = 0x2,
       C = 0x4,
       NC = 0x8,
       ANY = 0xF
      };

   class BlockTransition;
   class BlockTransitions {
   public:
      typedef std::forward_list<const BlockTransition *> TransitionList;
      const TransitionList& list() const { return list_; }

      BlockTransitions(const TransitionList& list);
      
   protected:
      const TransitionList list_;
   };

   class Block {
   public:
      typedef std::vector<Instruction> InstrVec;
      const Label *label() const { return label_; }
      const InstrVec& instrs() const { return instrs_; }
      const BlockTransitions& transitions() const { return transitions_; }
      
      /**
       * Constructor allows direct initialization of instructions vector.
       */
      template <typename... Args>
      Block(const Label *label, const BlockTransitions& transitions, Args... args):
         label_(label), transitions_(transitions), instrs_(args...) {}
      
   protected:
      const Label *label_;
      InstrVec instrs_;
      BlockTransitions transitions_;
   };

   class BlockTransition {
   public:
      Cond cond() const { return cond_; }
      
   protected:
      const Cond cond_;

      BlockTransition(Cond cond): cond_(cond) {}
   };

   class JumpTransition: public BlockTransition {
   public:
      const Block *dst() const { return dst_; }

      template <typename... Args>
      JumpTransition(const Block *dst, Args... args): BlockTransition(args...), dst_(dst) {}
      
   protected:
      const Block *dst_;
   };

   class ReturnTransition: public BlockTransition {
   public:
      template <typename... Args>
      ReturnTransition(Args... args): BlockTransition(args...) {}
   protected:
   };

   class FunctionImpl {
   public:
      const Block *entry() const { return entry_; }
      const MemoryLocation *loc() const { return loc_; }

      FunctionImpl(const Block *entry);
      
   protected:
      const Block *entry_;
      const MemoryLocation *loc_;
   };

   /* map IDs to indices on stack
    * should provide methods that translate ID to MemoryLocation
    */
   class CgenStackFrame {
   public:
      typedef std::unordered_map<const Symbol *, const MemoryValue *> SymMap;
      typedef std::vector<const Symbol *> ArgVec;

      /**
       * Access variables in stack frame.
       */
      const MemoryValue *val(const Symbol *sym) const;

      /**
       * Access argument value by index.
       */
      const MemoryValue *argval(int i) const;

      /**
       * Add local variable to stack frame.
       */
      void AddLocal(const ASTType *type);

      CgenStackFrame(const FunctionType *fn_type);
      
   protected:
      SymMap symmap_;
      ArgVec argvec_;
      const MemoryValue *next_local_;
   };
   
   class CgenEnv: public Env<SymInfo> {
   public:
      CgenEnv(): Env<SymInfo>() {}
      
   protected:
      
   };




}

#endif
