#include "asm.hpp"
#include "ast.hpp"

namespace zc::z80 {

   int bytes(Size size) {
      switch (size) {
      case Size::SZ_CHAR: return byte_size;
      case Size::SZ_SHORT: return word_size;
      case Size::SZ_INT:
      case Size::SZ_LONG:
      case Size::SZ_LONG_LONG:
      case Size::SZ_POINTER:
         return long_size;
      }
   }

   int bytes(const ASTType *type) {
      return bytes(type->size());
   }
   
}
