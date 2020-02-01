#ifndef __ASM_HPP
#error "include \"asm.hpp\""
#endif

#ifndef __ASM_MEM_HPP
#define __ASM_MEM_HPP

#include <string>
#include <vector>
#include <variant>

namespace zc::z80 {

   /**********
    * LABELS *
    **********/

   class Label {
   public:
      const std::string& name() const { return name_; }

      Label *Append(const std::string& suffix) const;
      Label *Prepend(const std::string& prefix) const;

      void EmitRef(std::ostream& os) const;
      void EmitDef(std::ostream& os) const;

      bool Eq(const Label *other) const { return name() == other->name(); }

      Label(const std::string& name): name_(name) {}

   protected:
      const std::string name_;
   };

   /*******************
    * MEMORY LOCATION *
    *******************/

#if 0
   class Value;
   /**
    * Base class representing a memory location.
    */
   class MemoryLocation {
   public:
      const Value *addr() const { return addr_; }

      MemoryLocation *Advance(const intmax_t& offset) const;
      void Emit(std::ostream& os) const;

      bool Eq(const MemoryLocation *other) const;

      MemoryLocation(const Value *addr): addr_(addr) {}
      
   protected:
      const Value *addr_;
   };
#endif

   
}

#endif