#ifndef __CGEN_HPP
#define __CGEN_HPP

#include "ast.hpp"
#include "asm.hpp"
#include "scopedtab.hpp"

namespace zc {

   class SymInfo {
   public:
      const z80::GenericMemoryLocation *loc() const { return loc_; }
      const ASTType *type() const { return type_; }

      SymInfo(const z80::GenericMemoryLocation *loc, const ASTType *type): loc_(loc), type_(type) {}
      
   protected:
      const z80::GenericMemoryLocation *loc_;
      const ASTType *type_;
   };
   
   class CgenEnv: public Env<SymInfo> {
   public:
      CgenEnv(): Env<SymInfo>() {}

   protected:
   };
   
}

#endif
