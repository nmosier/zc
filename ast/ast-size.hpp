#ifndef __AST_HPP
#error "include \"ast.hpp\" directly"
#endif

#ifndef __AST_SIZE_HPP
#define __AST_SIZE_HPP

namespace zc {

   enum class Size
      {SZ_CHAR,
       SZ_SHORT,
       SZ_INT,
       SZ_LONG,
       SZ_LONG_LONG,
       SZ_POINTER,
      };

   Size size(TypeSpec type_spec);
}

#endif
