#ifndef __ENV_HPP
#define __ENV_HPP

#include "scopedtab.hpp"
#include "symtab.hpp"

namespace zc {

   class ASTType;
   class SemantError;
   
   /**
    * Class representing basic single-symbol environment.
    */
   template <class ScopedSymtab>
   class SymbolEnv {
   public:
      SymbolEnv(ScopedSymtab& symtab): symtab_(symtab), sym_(nullptr) {}
      
      void Enter(Symbol *sym) {
         sym_ = sym;
      }
      
      const ASTType *Type() {
         return symtab_.Lookup(sym_);
      }
      
      void Exit() {
         sym_ = nullptr;
      }

   private:
      Symbol *sym_;
      ScopedSymtab& symtab_;
   };

   template <typename SymtabValue>
   class Env {
   public:
      typedef ScopedTable<Symbol *, SymtabValue> ScopedSymtab;
      ScopedSymtab& symtab() { return symtab_; }
      SymbolEnv<ScopedSymtab>& ext_env() { return ext_env_; }

   protected:
      ScopedSymtab symtab_;
      SymbolEnv<ScopedSymtab> ext_env_;
      
      Env(): symtab_(), ext_env_(symtab_) {}
   };

}

#endif
