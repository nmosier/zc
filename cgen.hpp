#ifndef __CGEN_HPP
#define __CGEN_HPP

#include <vector>
#include <deque>
#include <unordered_set>


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
      int bytes() const;
      int locals_bytes() const { return locals_bytes_; }

      void add_local(const VarDeclaration *type);
      void add_local(int bs) { locals_bytes_ += bs; }

      VarSymInfo *next_arg(const VarDeclaration *type);
      VarSymInfo *next_local(const VarDeclaration *type);

      StackFrame();
      StackFrame(const VarDeclarations *params);
      
   protected:
      int base_bytes_;
      int locals_bytes_;
      int args_bytes_;
      const Value *next_local_addr_;
      const Value *next_arg_addr_;
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
   
   class BlockTransition;
   class BlockTransitions {
   public:
      typedef std::vector<BlockTransition *> Transitions;
      const Transitions& vec() const { return vec_; }
      Transitions& vec() { return vec_; }

      bool live() const;
      void DumpAsm(std::ostream& os, Blocks& emitted_blocks, const FunctionImpl *impl) const;
      
      BlockTransitions(const Transitions& vec);
      BlockTransitions(): vec_() {}
      
   protected:
      Transitions vec_;
   };


   class BlockTransition {
   public:
      Cond cond() const { return cond_; }
      virtual Block *dst() const = 0;

      virtual void DumpAsm(std::ostream& os, const FunctionImpl *impl) const = 0;
      
   protected:
      const Cond cond_;

      BlockTransition(Cond cond): cond_(cond) {}
   };

   class JumpTransition: public BlockTransition {
   public:
      virtual Block *dst() const override { return dst_; }

      virtual void DumpAsm(std::ostream& os, const FunctionImpl *impl) const override;

      template <typename... Args>
      JumpTransition(Block *dst, Args... args): BlockTransition(args...), dst_(dst) {}
      
   protected:
      Block *dst_;
   };

   class ReturnTransition: public BlockTransition {
   public:
      virtual Block *dst() const override { return nullptr; }
      virtual void DumpAsm(std::ostream& os, const FunctionImpl *impl) const override;
      
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
      void DumpAsm(std::ostream& os,
                   std::unordered_set<const Block *>& emitted_blocks,
                   const FunctionImpl *impl) const;
      static void DumpAsm(const Block *block, std::ostream& os, const FunctionImpl *impl);

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
      
      FunctionImpl(const CgenEnv& env, Block *entry, Block *fin);
      
   protected:
      Block *entry_;
      Block *fin_;
      const LabelValue *addr_;
      int frame_bytes_;
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

   const Register *return_register(int bytes);
   Label *new_label();
   Label *new_label(const std::string& prefix);

   /**
    * Check whether expression is nonzero (i.e. evaluates to true as a predicate).
    * Zero flag (ZF) is set accordingly.
    */
   void emit_nonzero_test(CgenEnv& env, Block *block, int bytes);

   /**
    * Emit logical not on expression.
    */
   void emit_logical_not(CgenEnv& env, Block *block, int bytes);

   /**
    * Emit code that converts integral value into boolean (0 or 1).
    */
   void emit_booleanize(CgenEnv& env, Block *block, int bytes);

   /**
    * Generic emission routine for performing binary operation on two integers.
    * Post condition: lhs in %a or %hl; hs in %b or %de, depending on size.
    */
   Block *emit_binop(CgenEnv& env, Block *block, ASTBinaryExpr *expr);
   Block *emit_binop(CgenEnv& env, Block *block, ASTBinaryExpr *expr,
                     const RegisterValue *long_rhs_reg);
   
   /**
    * Emit instructions that move the contents of the zero flag (ZF) into register %a.
    */
   Block *emit_ld_a_zf(CgenEnv& env, Block *block, bool inverted = false);

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
