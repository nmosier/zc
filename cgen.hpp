#ifndef __CGEN_HPP
#define __CGEN_HPP

#include <vector>
#include <deque>
#include <unordered_set>
#include <numeric>

#include "asm.hpp"
#include "ast.hpp"
#include "scopedtab.hpp"

namespace zc {

   using namespace zc::z80;

   class Block;
   typedef std::unordered_set<Block *> Blocks;

   /**
    * Entry point to code generator.
    */
   void Cgen(TranslationUnit *root, std::ostream& os, const char *filename);

   class StatInfo {
   public:
      ASTStat *stat() const { return stat_; }
      Block *enter() const { return enter_; }
      Block *exit() const { return exit_; }

      StatInfo(ASTStat *stat, Block *enter, Block *exit): stat_(stat), enter_(enter), exit_(exit) {}
      
   private:
      ASTStat *stat_;
      Block *enter_;
      Block *exit_;
   };
   
   class SymInfo {
   public:
      enum class Kind {SYM_CONST, SYM_VAR};
      virtual Kind kind() const = 0;
      
      const VarDeclaration *decl() const { return decl_; }
      const Value *val() const { return val_; }

   protected:
      const VarDeclaration *decl_;
      const Value *val_;      

      SymInfo(): decl_(nullptr), val_(nullptr) {}
      SymInfo(const Value *val, const VarDeclaration *decl): decl_(decl), val_(val) {}
   };

   class ConstSymInfo: public SymInfo {
   public:
      virtual Kind kind() const override { return Kind::SYM_CONST; }

      template <typename... Args>
      ConstSymInfo(Args... args): SymInfo(args...) {}
      
   private:
   };

   class VarSymInfo: public SymInfo {
   public:
      virtual Kind kind() const override { return Kind::SYM_VAR; }

      const Value *addr() const { return addr_; }

      VarSymInfo(const Value *addr, const VarDeclaration *decl);
      template <typename... Args>
      VarSymInfo(const Value *addr, Args... args): SymInfo(args...), addr_(addr) {}
      VarSymInfo(const ExternalDecl *ext_decl);

   private:
      const Value *addr_;
   };

   class StackFrame {
   public:
      typedef std::list<int8_t> FrameIndices;

      const FrameValue *callee_bytes();
      const FrameValue *neg_callee_bytes();

      const FrameValue *saved_fp();
      const FrameValue *saved_ra();
      
      VarSymInfo *next_arg(const VarDeclaration *type);
      VarSymInfo *next_local(const VarDeclaration *type);
      const Value *next_tmp(const Value *tmp);

      StackFrame();
      StackFrame(const VarDeclarations *params);
      
   protected:
      FrameIndices *sizes_; /*!< list of sizes */
      FrameIndices::iterator saved_fp_; /*!< caller's frame pointer */
      FrameIndices::iterator saved_ra_; /*!< return address */
   };
   

   class CgenExtEnv {
   public:
      Symbol *sym() const { return sym_env_.sym(); }
      const StackFrame& frame() const { return frame_; }
      StackFrame& frame()  { return frame_; }

      void Enter(Symbol *sym, const VarDeclarations *args);
      void Exit();

      LabelValue *LabelGen(const Symbol *id) const;

   private:
      SymbolEnv sym_env_;
      StackFrame frame_;
   };

   enum class Cond {Z, NZ, C, NC, ANY};
   std::ostream& operator<<(std::ostream& os, Cond cond);
   Cond invert(Cond cond);
   
   class BlockTransition;
   class BlockTransitions {
   public:
      typedef std::vector<BlockTransition *> Transitions;
      const Transitions& vec() const { return vec_; }
      Transitions& vec() { return vec_; }

      bool live() const;
      void DumpAsm(std::ostream& os, Blocks& visited) const;
      void Prune();
      void Serialize(Instructions& out, Blocks& visited);
      
      BlockTransitions(const Transitions& vec);
      BlockTransitions(): vec_() {}
      
   protected:
      Transitions vec_;
   };


   class BlockTransition {
   public:
      Cond cond() const { return cond_; }
      virtual Block *dst() const = 0;
      
      virtual BlockTransition *Resolve(const FunctionImpl *impl) = 0;
      virtual void Serialize(Instructions& out) = 0;
      virtual void DumpAsm(std::ostream& os) const = 0;
      
   protected:
      const Cond cond_;

      BlockTransition(Cond cond): cond_(cond) {}
   };

   class JumpTransition: public BlockTransition {
   public:
      virtual Block *dst() const override { return dst_; }

      virtual BlockTransition *Resolve(const FunctionImpl *impl) override { return this; }
      virtual void Serialize(Instructions& out) override;
      virtual void DumpAsm(std::ostream& os) const override;

      template <typename... Args>
      JumpTransition(Block *dst, Args... args): BlockTransition(args...), dst_(dst) {}
      
   protected:
      Block *dst_;
   };

   class ReturnTransition: public BlockTransition {
   public:
      virtual Block *dst() const override { return nullptr; }

      virtual BlockTransition *Resolve(const FunctionImpl *impl) override;
      virtual void Serialize(Instructions& out) override {
         throw std::logic_error("attempt to serialize return transition ");
      }
      virtual void DumpAsm(std::ostream& os) const override {
         throw std::logic_error("return transition should have been resolved");
      }
      
      template <typename... Args>
      ReturnTransition(Args... args): BlockTransition(args...) {}
   protected:
   };

   
   class Block {
   public:
      const Label *label() const { return label_; }
      const Instructions& instrs() const { return instrs_; }
      Instructions& instrs() { return instrs_; }
      const BlockTransitions& transitions() const { return transitions_; }
      BlockTransitions& transitions() { return transitions_; }

      bool live() const { return transitions().live(); }

      
      /**
       * Emit as assembly.
       * @param os output stream
       * @param emitted_blocks set of blocks that have already been emitted (to avoid duplication).
       */
      static void DumpAsm(Block *block, std::ostream& os, Blocks& visited);

      /**
       * Resolve block's instructions and transitions.
       */
      static void Resolve(Block *block, const FunctionImpl *impl);

      /**
       * Serialize instructions.
       */
      static void Serialize(Block *block, Instructions& out, Blocks& visited);
      
      /**
       * Apply function to each block.
       * @tparam Func type of functor to apply. `this' is always passed as first argument.
       * @tparam Args arguments to pass to each invokation
       * @param func functor to apply to each block
       * @param args arguments to pass to each invokation
       */
      template <typename Func, typename... Args>
      void for_each_block(Blocks& visited, Func func, Args&&... args) {
         /* check if visited */
         if (visited.find(this) != visited.end()) { return; }
         visited.insert(this);

         /* apply function */
         func(this, args...);

         /* visit transitions */
         for (const BlockTransition *trans : transitions().vec()) {
            if (trans->dst() == nullptr) { continue; }
            trans->dst()->for_each_block(visited, func, args...);
         }
      }
      
      /**
       * Constructor allows direct initialization of instructions vector.
       */
      template <typename... Args>
      Block(const Label *label, const BlockTransitions& transitions, Args... args):
         label_(label), transitions_(transitions), instrs_(args...) {}

      template <typename... Args>
      Block(const Label *label, Args... args): label_(label), transitions_(args...) {}

      Block() {}
      
   protected:
      const Label *label_;
      Instructions instrs_;
      BlockTransitions transitions_;

   };

   class FunctionImpl {
   public:
      Block *entry() const { return entry_; }
      Block *fin() const { return fin_; }
      const LabelValue *addr() const { return addr_; }
      
      void DumpAsm(std::ostream& os) const;
      void Resolve();
      void Serialize();

      FunctionImpl(CgenEnv& env, Block *entry, Block *fin);
      
   protected:
      Block *entry_;
      Block *fin_;
      const LabelValue *addr_;
      StackFrame& stack_frame_;
      std::optional<Instructions> instrs_ = std::nullopt; /*!< only set after serialization */
   };

   class FunctionImpls {
      typedef std::vector<FunctionImpl> Impls;
   public:
      Impls& impls() { return impls_; }
      const Impls& impls() const { return impls_; }

      void DumpAsm(std::ostream& os) const;
      
      template <typename... Args>
      FunctionImpls(Args... args): impls_(args...) {}
      
   private:
      Impls impls_;
   };

   class StringConstants {
      typedef std::unordered_map<std::string, const Label *> Strings;
   public:
      void Insert(const std::string& str);
      LabelValue *Ref(const std::string& str);
      void DumpAsm(std::ostream& os) const;
      
      StringConstants(): counter_(0), strs_() {}
      
   private:
      int counter_;
      Strings strs_;

      Label *new_label();
   };

   
   class CgenEnv: public Env<SymInfo, TaggedType, StatInfo, CgenExtEnv> {
   public:
      const StringConstants& strconsts() const { return strconsts_; }
      StringConstants& strconsts() { return strconsts_; }
      const FunctionImpls& impls() const { return impls_; }
      FunctionImpls& impls() { return impls_; }

      CgenEnv(): Env<SymInfo, TaggedType, StatInfo, CgenExtEnv>(), strconsts_(), impls_() {}

      void DumpAsm(std::ostream& os) const;
      void Resolve();
      void Serialize();
      
   protected:
      /**
       * String constants.
       */
      StringConstants strconsts_;

      /**
       * Function implementations.
       */
      FunctionImpls impls_;
   };

   /**
    * Get register that return value should be held in given value size.
    */
   const Register *return_register(const Value *val);
   const Register *return_register(int bytes);
   const RegisterValue *return_rv(const Value *val);
   const RegisterValue *return_rv(int bytes);

   /**
    * Get register arguments for CRT routines.
    */
   const RegisterValue *crt_arg1(const Value *val);
   const RegisterValue *crt_arg1(int bytes);
   const RegisterValue *crt_arg2(const Value *val);
   const RegisterValue *crt_arg2(int bytes);

   /**
    * Get prefix for CRT routines given value size.
    */
   std::string crt_prefix(const Value *val);
   std::string crt_prefix(int bytes);

   /**
    * Get accumulator register for given size.
    */
   const RegisterValue *accumulator(const Value *val);
   const RegisterValue *accumulator(int bytes);
   
   Label *new_label();
   Label *new_label(const std::string& prefix);

   /**
    * Check whether expression is nonzero (i.e. evaluates to true as a predicate).
    * @param in value to test
    * @param output flags value
    */
   void emit_nonzero_test(CgenEnv& env, Block *block, const Value *in, const FlagValue **out);

   /**
    * Emit logical not on expression.
    */
   void emit_logical_not(CgenEnv& env, Block *block, const Value *in, const Value **out);

   /**
    * Emit code that converts integral value into boolean (0 or 1).
    * NOTE: Places result in byte register %a.
    */
   // void emit_booleanize(CgenEnv& env, Block *block, const Value *in);

   // void emit_booleanize_flag(CgenEnv& env, Block *block, const FlagValue *in);

   void emit_booleanize_flag_byte(CgenEnv& env, Block *block, const FlagValue *in,
                                  const Value **out);
   void emit_booleanize_flag_long(CgenEnv& env, Block *block, const FlagValue *in,
                                  const Value **out);
   
   
   /**
    * Emit code for predicate.
    */ 
   void emit_predicate(CgenEnv& env, Block *block, ASTExpr *expr, Block *take, Block *leave);  

   /**
    * Emit increment/decrement.
    * @param env codegen environment
    * @param block block to write instructions to
    * @param inc_not_dec whether to increment or decrement
    * @param pre_not_post whether output value should reflect update
    * @param subexpr subexpression to generate
    * @param out output value
    * @return continuation block
    */
   Block *emit_incdec(CgenEnv& env, Block *block, bool inc_not_dec, bool pre_not_post,
                      ASTExpr *subexpr, const Value **out);
   
   /**
    * Generic emission routine for performing binary operation on two integers.
    */
   Block *emit_binop(CgenEnv& env, Block *block, ASTBinaryExpr *expr,
                     const Value **out_lhs, const Value **out_rhs);

   /**
    * Emit CRT frameset.
    */
   void emit_frameset(CgenEnv& env, Block *block);

   /**
    * Emit CRT frameunset.
    */
   void emit_frameunset(CgenEnv& env, Block *block);

   /**
    * Emit call to CRT routine.
    */
   void emit_crt(const std::string& name, Block *block);
   
   /*** C RUNTIME ***/
   const Label crt_l_indcall("__indcall");
   const LabelValue crt_lv_indcall(&crt_l_indcall);
   
}

#endif
