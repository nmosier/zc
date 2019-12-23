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
      
      static TranslationUnit *Create(ExternalDecls *decls, SourceLoc& loc) { return new TranslationUnit(decls, loc); }

      virtual void DumpNode(std::ostream& os) const override { os << "TranslationUnit"; }

      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {
         decls()->Dump(os, level, with_types);
      }

      virtual void TypeCheck(SemantEnv& env, bool scoped = true) override;
      
   protected:
      ExternalDecls *decls_;
      
      TranslationUnit(ExternalDecls *decls, SourceLoc& loc): ASTNode(loc), decls_(decls) {}
   };
   
}

#endif
