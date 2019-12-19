/* ast.h -- AST */

#ifndef __AST_H
#define __AST_H

#include <string>
#include <vector>
#include <variant>
#include <type_traits>
#include <stdint.h>

#include "ast/ast-fwd.h"

#include "ast/ast-base.h"
#include "ast/ast-expr.h"
#include "ast/ast-decl.h"
#include "ast/ast-stat.h"

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


   class ID: public ASTNode {
   public:
      const std::string& id() const { return id_; }

      static ID *Create(std::string& id, SourceLoc& loc) { return new ID(id, loc); }
      
   protected:
      std::string id_;

      ID(std::string& id, SourceLoc& loc): ASTNode(loc), id_(id) {}
   };

   class ParamTypeList: public ASTNode {
   public:
      ParamDecls *decls() const { return decls_; }

      static ParamTypeList *Create(ParamDecls *decls, SourceLoc& loc)
      { return new ParamTypeList(decls, loc); }
      
   protected:
      ParamDecls *decls_;

      ParamTypeList(ParamDecls *decls, SourceLoc& loc): ASTNode(loc), decls_(decls) {}
   };



   /************************
    *     EXPRESSIONS      *
    ************************/



   class UnaryOperator: public ASTOp {
   public:
      enum Kind {AND, DEREFERENCE, PLUS, MINUS, BITWISE_NOT, LOGICAL_NOT};
      const Kind& kind() const { return kind_; }

      static UnaryOperator *Create(Kind kind, SourceLoc& loc)
      { return new UnaryOperator(kind, loc); }
      
   protected:
      Kind kind_;

      UnaryOperator(Kind kind, SourceLoc& loc): ASTOp(loc), kind_(kind) {}
   };

   
} // namespace zc

#endif
