#ifndef __OPTIM_HPP
#define __OPTIM_HPP

#include <unordered_map>

#include "ast.hpp"

namespace zc {

   struct CgenOptimInfo {
      /* AST optimization flags */
      bool reduce_const = true;
      
      /* Register allocation flags */
      bool join_vars = true;

      typedef std::unordered_map< std::string, bool CgenOptimInfo::* > NameTable;
      static NameTable nametab;
      
      /**
       * Clears all optimization options present.
       */ 
      void clear_all() {
         for (auto pair : nametab) {
            this->*(pair.second) = false;
         }
      }

      /**
       * Set all optimization flags.
       */
      void set_all() {
         for (auto pair : nametab) {
            this->*(pair.second) = true;
         }
      }

      template <typename Err, typename Help>
      int set_flags(const char *flags, Err err, Help help) {
         char *buf = strdup(flags);
         assert(buf);
         
         int retv = 0;
         const char *flag;
         while ((flag = strsep(&buf, ",")) != NULL) {
            if (set_flag(flag, err, help) < 0) {
               retv = -1;
            }
         }
         
         return retv;
      }

      template <typename Err, typename Help>
      int set_flag(const char *flag, Err err, Help help) {
         /* check for special flags: `all', `none' */
         if (strcmp(flag, "all") == 0) {
            set_all();
            return 0;
         }
         if (strcmp(flag, "none") == 0) {
            clear_all();
            return 0;
         }
         if (strcmp(flag, "help") == 0) {
            for (auto pair : nametab) {
               help(pair.first.c_str());
            }
            return -1;
         }
      
         const char *no = "no-";
         auto it = nametab.find(flag);
         if (it != nametab.end()) {
            this->*(it->second) = true;
            return 0;
         }

         /* search for `no-' */
         if (strncmp(flag, no, strlen(no)) == 0) {
            const char *no_flag = flag + strlen(no);
            auto it = nametab.find(no_flag);
            if (it != nametab.end()) {
               this->*(it->second) = false;
               return 0;
            }
         }

         err(flag);
         return -1;
      }

   private:
   };
   extern CgenOptimInfo g_optim;
   
   void OptimizeAST(TranslationUnit *root);
   void OptimizeIR(CgenEnv& env);
}

#endif
