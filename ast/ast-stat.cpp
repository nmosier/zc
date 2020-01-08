#include "ast.hpp"

namespace zc {

   void WhileStat::DumpChildren(std::ostream& os, int level, bool with_types) const {
      pred()->Dump(os, level, with_types);
      body()->Dump(os, level, with_types);
   }

   void CompoundStat::DumpChildren(std::ostream& os, int level, bool with_types) const {
      decls_->Dump(os, level, with_types);
      stats_->Dump(os, level, with_types);
   }
   
   
}
