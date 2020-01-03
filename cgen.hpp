#ifndef __CGEN_HPP
#define __CGEN_HPP

#include <vector>

#include "asm.hpp"
#include "ast.hpp"
#include "scopedtab.hpp"

namespace zc {

   using namespace zc::z80;

   /**
    * Entry point to code generator.
    */
   void Cgen(TranslationUnit *root, std::ostream& os, const char *filename);

   
   class SymInfo {
   public:
      const ASTType *type() const { return type_; }

      /**
       * The lvalue of this symbol. If it is a variable, it points to its storage location.
       * If it's a function, then it is the address of the function.
       */
      const Value *lval() const { return lval_; }

      /**
       * The rvalue of this symbol. If it is a variable, it represents the contents of its storage
       * location.
       */
      const Value *rval() const { return rval_; }

      SymInfo(const ASTType *type, const Value *lval);
      SymInfo(const ExternalDecl *ext_decl);
      // SymInfo(const Decl *decl);
      
   protected:
      const ASTType *type_;
      const Value *lval_;
      const Value *rval_;
   };

   class CgenExtEnv {
   public:
      Symbol *sym() const { return sym_env_.sym(); }

      void Enter(Symbol *sym);
      void Exit();

      const MemoryValue *NewLocal(Size size);
      
   private:
      SymbolEnv sym_env_;
      const MemoryValue *next_local_;
   };

   enum class Cond {Z, NZ, C, NC, ANY};
   
   class BlockTransition;
   class BlockTransitions {
   public:
      typedef std::vector<BlockTransition *> Transitions;
      const Transitions& vec() const { return vec_; }
      Transitions& vec() { return vec_; }

      bool live() const;
      
      BlockTransitions(const Transitions& vec);
      BlockTransitions(): vec_() {}
      
   protected:
      Transitions vec_;
   };

   class Block {
   public:
      typedef std::vector<Instruction> InstrVec;
      const Label *label() const { return label_; }
      const InstrVec& instrs() const { return instrs_; }
      InstrVec& instrs() { return instrs_; }
      const BlockTransitions& transitions() const { return transitions_; }
      BlockTransitions& transitions() { return transitions_; }

      bool live() const { return transitions().live(); }
      
      /**
       * Constructor allows direct initialization of instructions vector.
       */
      template <typename... Args>
      Block(const Label *label, const BlockTransitions& transitions, Args... args):
         label_(label), transitions_(transitions), instrs_(args...) {}

      template <typename... Args>
      Block(const Label *label, Args... args): transitions_(args...) {}
      
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
      const LabelValue *addr() const { return addr_; }

      FunctionImpl(): entry_(nullptr), addr_(nullptr) {}
      FunctionImpl(const Block *entry);
      
   protected:
      const Block *entry_;
      const LabelValue *addr_;
   };

   class StringConstants {
      typedef std::unordered_map<std::string, const Label *> Strings;
   public:
      void Insert(const std::string& str);
      LabelValue *Ref(const std::string& str);
      
      StringConstants(): counter_(0), strs_() {}
      
   private:
      int counter_;
      Strings strs_;

      Label *new_label();
   };
   
   class CgenEnv: public Env<SymInfo,CgenExtEnv> {
   public:
      const MemoryValue *next_local() const { return next_local_; }
      const StringConstants& strconsts() const { return strconsts_; }
      StringConstants& strconsts() { return strconsts_; }
      
      CgenEnv(): Env<SymInfo,CgenExtEnv>(), next_local_(&FP_memval), strconsts_() {}
      
   protected:
      /**
       * Track the memory value of the most recently allocated local.
       */
      const MemoryValue *next_local_;

      StringConstants strconsts_;
   };


   const Register *return_register(Size sz);
   Label *new_label();
   Label *new_label(const std::string& prefix);

   /**
    * Check whether expression is nonzero (i.e. evaluates to true as a predicate).
    * Zero flag (ZF) is set accordingly.
    */
   void emit_nonzero_test(CgenEnv& env, Block *block, Size size);

   /**
    * Emit logical not on expression.
    */
   void emit_logical_not(CgenEnv& env, Block *block, Size size);

   /**
    * Emit code that converts integral value into boolean (0 or 1).
    */
   void emit_booleanize(CgenEnv& env, Block *block, Size size);

   /**
    * Generic emission routine for performing binary operation on two integers.
    * Post condition: lhs in %a or %hl; hs in %b or %de, depending on size.
    */
   void emit_binop(CgenEnv& env, Block *block, ASTBinaryExpr *expr);
   
   /**
    * Emit instructions that move the contents of the zero flag (ZF) into register %a.
    */
   Block *emit_ld_a_zf(CgenEnv& env, Block *block, bool inverted = false);   

   /*** C RUNTIME ***/
   const Label crt_l_call("call");
   const LabelValue crt_lv_call(&crt_l_call);
   
}

#endif