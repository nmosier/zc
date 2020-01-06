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

   /**
    * Entry point to code generator.
    */
   void Cgen(TranslationUnit *root, std::ostream& os, const char *filename);

   
   class SymInfo {
   public:
      const ASTType *type() const { return type_; }

      /**
       * The lvalue and rvalue of this symbol. If it is a variable, it points to its storage
       * location. If it's a function, then it is the address of the function.
       */
      const Value *val() const { return val_; }

      /**
       * (Optional) The address of this symbol.
       */
      const Value *addr() const { return addr_; }

      SymInfo(const ExternalDecl *ext_decl);
      SymInfo(const ASTType *type, const Value *val, const Value *addr);
      SymInfo(const ASTType *type, const Value *addr);
      
   protected:
      const ASTType *type_;
      const Value *val_;
      const Value *addr_;
   };

   class StackFrame {
   public:
      int bytes() const;
      int locals_bytes() const { return locals_bytes_; }

      void add_local(const ASTType *type);
      void add_local(Size sz);
      void add_local(int bs) { locals_bytes_ += bs; }

      SymInfo *next_arg(const ASTType *type);
      SymInfo *next_local(const ASTType *type);

      StackFrame();
      StackFrame(const Types *params);
      
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

      void Enter(Symbol *sym, const Types *args);
      void Exit();

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
      void DumpAsm(std::ostream& os, std::unordered_set<const Block *>& emitted_blocks) const;
      
      BlockTransitions(const Transitions& vec);
      BlockTransitions(): vec_() {}
      
   protected:
      Transitions vec_;
   };

   class Block {
   public:
      typedef std::deque<Instruction *> InstrVec;
      const Label *label() const { return label_; }
      const InstrVec& instrs() const { return instrs_; }
      InstrVec& instrs() { return instrs_; }
      const BlockTransitions& transitions() const { return transitions_; }
      BlockTransitions& transitions() { return transitions_; }

      bool live() const { return transitions().live(); }

      /**
       * Emit as assembly.
       * @param os output stream
       * @param emitted_blocks set of blocks that have already been emitted (to avoid duplication).
       */
      void DumpAsm(std::ostream& os, std::unordered_set<const Block *>& emitted_blocks) const;
      
      /**
       * Constructor allows direct initialization of instructions vector.
       */
      template <typename... Args>
      Block(const Label *label, const BlockTransitions& transitions, Args... args):
         label_(label), transitions_(transitions), instrs_(args...) {}

      template <typename... Args>
      Block(const Label *label, Args... args): label_(label), transitions_(args...) {}
      
   protected:
      const Label *label_;
      InstrVec instrs_;
      BlockTransitions transitions_;

   };

   class BlockTransition {
   public:
      Cond cond() const { return cond_; }

      virtual void DumpAsm(std::ostream& os,
                           std::unordered_set<const Block *>& to_emit) const = 0;
      
   protected:
      const Cond cond_;

      BlockTransition(Cond cond): cond_(cond) {}
   };

   class JumpTransition: public BlockTransition {
   public:
      const Block *dst() const { return dst_; }

      virtual void DumpAsm(std::ostream& os,
                           std::unordered_set<const Block *>& to_emit) const override;

      template <typename... Args>
      JumpTransition(const Block *dst, Args... args): BlockTransition(args...), dst_(dst) {}
      
   protected:
      const Block *dst_;
   };

   class ReturnTransition: public BlockTransition {
   public:
      virtual void DumpAsm(std::ostream& os,
                           std::unordered_set<const Block *>& to_emit) const override;
      
      template <typename... Args>
      ReturnTransition(Args... args): BlockTransition(args...) {}
   protected:
   };

   class FunctionImpl {
   public:
      const Block *entry() const { return entry_; }
      const LabelValue *addr() const { return addr_; }
      
      void DumpAsm(std::ostream& os) const;
      
      FunctionImpl(const CgenEnv& env, const Block *entry, const Block *fin);
      
   protected:
      const Block *entry_;
      const Block *fin_;
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

   
   class CgenEnv: public Env<SymInfo,CgenExtEnv> {
   public:
      const StringConstants& strconsts() const { return strconsts_; }
      StringConstants& strconsts() { return strconsts_; }
      const FunctionImpls& impls() const { return impls_; }
      FunctionImpls& impls() { return impls_; }

      CgenEnv(): Env<SymInfo,CgenExtEnv>(), strconsts_(), impls_() {}

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

   /**
    * Emit CRT frameset.
    */
   void emit_frameset(CgenEnv& env, Block *block);

   /**
    * Emit CRT frame unset.
    */
   void emit_frameunset(CgenEnv& env, Block *block);
   
   /*** C RUNTIME ***/
   const Label crt_l_call("call");
   const LabelValue crt_lv_call(&crt_l_call);
   
}

#endif
