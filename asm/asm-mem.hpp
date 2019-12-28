#ifndef __ASM_HPP
#error "include \"asm.hpp\""
#endif

#ifndef __ASM_MEM_HPP
#define __ASM_MEM_HPP

#include <string>
#include <vector>

namespace zc::z80 {



   /**********
    * LABELS *
    **********/

   class Label {
   public:
      const std::string& name() const { return name_; }

      Label(const std::string& name): name_(name) {}
      
   protected:
      const std::string name_;
   };

   /*******************
    * MEMORY LOCATION *
    *******************/

   template <Size sz> class Value;
   /**
    * Base class representing a memory location.
    */
   template <Size sz>
   class MemoryLocation {
   public:
      const Value<Size::LONG> *addr() const { return addr_; }

      MemoryLocation(const Value<Size::LONG> *addr): addr_(addr) {}
      
   protected:
      const Value<Size::LONG> *addr_;
   };
   
   

}

#endif
