#ifndef __AST_HPP
#error "include ast.hpp, not ast/ast-main.hpp directly"
#endif

#ifndef __AST_MAIN_HPP
#define __AST_MAIN_HPP

#include <iostream>

#include "ast-base.hpp"

namespace zc {

   class TranslationUnit: public ASTNode {
   public:
      ExternalDecls *decls() const { return decls_; }
      
      static TranslationUnit *Create(SourceLoc & loc) { return new TranslationUnit(loc); }

      virtual void DumpNode(std::ostream& os) const override { os << "TranslationUnit"; }

      virtual void DumpChildren(std::ostream& os, int level) const override {
         decls()->Dump(os, level);
      }
      
   protected:
      ExternalDecls *decls_;
      
      TranslationUnit(SourceLoc& loc): ASTNode(loc), decls_(nullptr) {}
      TranslationUnit(ExternalDecls *decls, SourceLoc& loc):
         ASTNode(loc), decls_(decls) {}
   };
   
}

#endif
