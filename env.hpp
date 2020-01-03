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
   class SymbolEnv {
   public:
      Symbol *sym() const { return sym_; }
      
      SymbolEnv(): sym_(nullptr) {}
      
      void Enter(Symbol *sym) {
         sym_ = sym;
      }
      
      void Exit() {
         sym_ = nullptr;
      }

   private:
      Symbol *sym_;
   };
   
   template <typename SymtabValue, class ExtEnv>
   class Env {
   public:
      typedef ScopedTable<Symbol *, SymtabValue> ScopedSymtab;
      ScopedSymtab& symtab() { return symtab_; }
      const ScopedSymtab& symtab() const { return symtab_; }
      
      ExtEnv& ext_env() { return ext_env_; }
      const ExtEnv& ext_env() const { return ext_env_; }
      
   protected:
      ScopedSymtab symtab_;
      ExtEnv ext_env_;
      
      Env(): symtab_(), ext_env_() {}
   };

}

#endif
