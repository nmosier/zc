#include <cassert>

#include "asm.hpp"

namespace zc::z80 {

   int Value::size(Kind kind) {
      switch (kind) {
      case Kind::V_BYTE: return byte_size();
      case Kind::V_WORD: return word_size();
      case Kind::V_LONG: return long_size();
      }
   }

   int Value::size() const {
      return size(kind());
   }

   template <typename... Args>
   MultibyteRegister::MultibyteRegister(ByteRegs regs, Args... args):
      Register(args...), regs_(regs) {
      assert(regs.size() == Value::size(Value::Kind::V_WORD) ||
             regs.size() == Value::size(Value::Kind::V_LONG));
   }
      
   
}
