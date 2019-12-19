/* ast.h -- AST */

#ifndef __AST_HPP
#define __AST_HPP

#include <string>
#include <vector>
#include <variant>
#include <type_traits>
#include <stdint.h>

#include "ast/ast-fwd.hpp"

#include "ast/ast-base.hpp"
#include "ast/ast-expr.hpp"
#include "ast/ast-decl.hpp"
#include "ast/ast-stat.hpp"

namespace zc {

   /* Abstract Classes */
   /* Classes that end with 'M' are inheritable members. */
   /* Classes that end with 'A' are abstract base classes. */
   
   


   /* Concrete AST Node Definitions  */
   
   class TranslationUnit: public ASTNode {
   public:
      ExternalDecls *decls() const { return decls_; }

      static TranslationUnit *Create(SourceLoc & loc) { return new TranslationUnit(loc); }
      
   protected:
      ExternalDecls *decls_;
      
      TranslationUnit(SourceLoc& loc): ASTNode(loc), decls_(nullptr) {}
      TranslationUnit(ExternalDecls *decls, SourceLoc& loc):
         ASTNode(loc), decls_(decls) {}
   };

   class Identifier: public ASTNode {
   public:
      const std::string& id() const { return id_; }

      static Identifier *Create(std::string& id, SourceLoc& loc) { return new Identifier(id, loc); }
      
   protected:
      std::string id_;
      
      Identifier(std::string& id, SourceLoc& loc): ASTNode(loc), id_(id) {}
   };

} // namespace zc

#endif
