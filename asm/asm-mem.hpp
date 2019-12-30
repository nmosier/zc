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

      void EmitRef(std::ostream& os) const;
      void EmitDef(std::ostream& os) const;

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

      void Emit(std::ostream& os) const;

      MemoryLocation(const Value<Size::LONG> *addr): addr_(addr) {}
      
   protected:
      const Value<Size::LONG> *addr_;
   };

   /**
    * generic, i.e. of undefined width
    */
   typedef std::variant<MemoryLocation<Size::BYTE>, MemoryLocation<Size::LONG>>
   MemoryLocationVariant;
   class GenericMemoryLocation: public MemoryLocationVariant {
   public:
      template <typename... Args>
      GenericMemoryLocation(Args... args): MemoryLocationVariant(args...) {}
      
   protected:
   };
   
}

#endif
