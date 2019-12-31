#include <numeric>

#include "ast.hpp"
#include "asm.hpp"

#include "cgen.hpp"

namespace zc {

   FunctionImpl::FunctionImpl(const Block *entry): entry_(entry) {
      loc_ = new MemoryLocation(new LabelValue(entry->label()));
   }

   const MemoryLocation *FunctionInfo::loc() const {
      return impl()->loc();
   }
   
   BlockTransitions::BlockTransitions(const TransitionList& list): list_(list) {
      /* get mask of conditions */
      int mask = std::accumulate(list.begin(), list.end(), 0,
                                 [](int acc, const BlockTransition *val) {
                                    return acc | (int) val->cond();
                                 });
      assert(mask == (int) Cond::ANY);
   }
   
   void TranslationUnit::CodeGen(CgenEnv& env) const {
      decls()->CodeGen(env);
   }

   void ExternalDecl::CodeGen(CgenEnv& env) const {
      /* 1. Obtain memory value for global variable.
       * 2. Map symbol to memory value in scope.
       */
      Symbol *sym = decl()->id()->id();
      ASTType *type = decl()->Type();
      
      
   }

   CgenStackFrame::CgenStackFrame(const FunctionType *fn_type):
      symmap_(), argvec_(), next_local_(nullptr) {
      /* add arguments to stack frame */
      /* create argument symbol vector */
      const Types *param_types = fn_type->params();
      argvec_.resize(param_types->vec().size());

      auto param_it = param_types->vec().rbegin();
      auto param_end = param_types->vec().rend();
      auto argvec_it = argvec_.rbegin();

      for (; param_it != param_end; ++param_it, ++argvec_it) {
         const Symbol *sym = (*param_it)->sym();
         
         /* enter symbol into argument vector */
         *argvec_it = sym;

         /* enter into table */
         int param_bytes = bytes(*param_it);
         symmap_[sym] = FP_memval.Next(param_bytes);
      }

      /* initialize next local */
      next_local_ = &FP_memval;
   }

   const MemoryValue *CgenStackFrame::val(const Symbol *sym) const {
      auto it = symmap_.find(sym);
      return (it == symmap_.end()) ? nullptr : it->second;
   }

   const MemoryValue *CgenStackFrame::argval(int i) const {
      if (i >= argvec_.size() || i < 0) {
         throw std::range_error("argument out of range");
      }
      return val(argvec_[i]);
   }

   void CgenStackFrame::AddLocal(const ASTType *type) {
      next_local_ = next_local_->Prev(bytes(type));
      symmap_[type->sym()] = next_local_;
   }
   
}
