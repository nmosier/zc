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

   /**
    * Table that tracks types, including structs and eventually typedefs, as they are declared.
    */
   template <typename TypeClass>
   class TypeTable {
   public:
      
      
   private:
      
      
   };

   template <typename SymtabValue, typename StructValue, class ExtEnv>
   class Env {
   public:
      typedef ScopedTable<Symbol *, SymtabValue> ScopedSymtab;
      typedef ScopedTable<Symbol *, StructValue> ScopedStructs;

      /* Accessors */
      ScopedSymtab& symtab() { return symtab_; }
      const ScopedSymtab& symtab() const { return symtab_; }
      
      ScopedStructs& structs() { return structs_; }
      const ScopedStructs& structs() const { return structs_; }
      
      ExtEnv& ext_env() { return ext_env_; }
      const ExtEnv& ext_env() const { return ext_env_; }

      /* Scoping */
      void EnterScope();
      void ExitScope();
      
   protected:
      ScopedSymtab symtab_;
      ScopedStructs structs_;
      ExtEnv ext_env_;
      
      Env(): symtab_(), structs_(), ext_env_() {}
   };

}

#endif
