#include "ast.hpp"

namespace zc {

   Size size(TypeSpec type_spec) {
      switch (type_spec) {
      case TypeSpec::TYPE_VOID:      throw std::logic_error("attempted to take size of 'void'");
      case TypeSpec::TYPE_CHAR:      return Size::SZ_CHAR;
      case TypeSpec::TYPE_SHORT:     return Size::SZ_SHORT;
      case TypeSpec::TYPE_INT:       return Size::SZ_INT;
      case TypeSpec::TYPE_LONG:      return Size::SZ_LONG;
      case TypeSpec::TYPE_LONG_LONG: return Size::SZ_LONG_LONG;
      }
    }
   
}
