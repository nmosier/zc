#ifndef __OPTIM_HPP
#define __OPTIM_HPP

#include <unordered_map>

#include "ast.hpp"
#include "config.hpp"

namespace zc {

   struct CgenOptimInfo: public Config<CgenOptimInfo> {
      /** AST optimization flags */      
      bool reduce_const = true;
      bool bool_flag = true;
      IntegralType::IntKind bool_spec() const {
         return bool_flag ? IntegralType::IntKind::SPEC_BOOL : IntegralType::IntKind::SPEC_CHAR;
      }
      IntegralType *bool_type() const { return IntegralType::Create(bool_spec(), 0); }
      
      /** Register allocation flags */
      bool join_vars = true;

      /** ASM flags */
      bool peephole = true;
      bool minimize_transitions = true;
      
      CgenOptimInfo(const NameTable& nametab): Config(nametab) {}

   };
   extern CgenOptimInfo g_optim;
   
   void OptimizeAST(TranslationUnit *root);
   void OptimizeIR(CgenEnv& env);
}

#endif
