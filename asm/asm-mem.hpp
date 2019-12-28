#ifndef __ASM_HPP
#error "include \"asm.hpp\""
#endif

#ifndef __ASM_MEM_HPP
#define __ASM_MEM_HPP

namespace zc::z80 {

   extern bool ez80_mode;
   
   class Register {
   public:
      const std::string& name() const { return name_; }
      virtual int size() const = 0;

      Register(const std::string& name): name_(name) {}
      
   protected:
      const std::string name_;
   };

   template <int N>
   class RegisterX: public Register {
   public:
      virtual int size() const override { return N; }

      template <typename... Args>
      RegisterX(Args... args): Register(args...) {}

   protected:
   };

   typedef RegisterX<1> Register8;
   typedef RegisterX<2> Register16;
   typedef RegisterX<3> Register24;
   
   class MemoryLocation {
   public:
   protected:
   };

   

}

#endif
