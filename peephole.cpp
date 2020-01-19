#include <list>

#include "peephole.hpp"
#include "asm.hpp"

namespace zc::z80 {

   void PeepholeOptimization::ReplaceAll() const {
      for (const_iterator it = input_instrs_.begin(), end = input_instrs_.end();
           it != end;
           ++it) {
         it = ReplaceAt(it);
      }
   }

   Instructions::const_iterator PeepholeOptimization::ReplaceAt(const_iterator it) const {
      Instructions new_instrs;
      const_iterator rm_end = replace_(it, input_instrs_.end(), new_instrs);
      const_iterator new_end = input_instrs_.erase(it, rm_end);
      input_instrs_.splice(new_end, new_instrs);
      return new_end;
   }

   /*** PEEPHOLE OPTIMIZATION FUNCTIONS ***/

   /* Indexed Register Load/Store
    * lea rr1,ix+*
    * ld (rr1),v | ld r2,(rr1)
    * ------------
    * ld (ix+*),v | ld r2,(ix+*)
    */
   Instructions::const_iterator peephole_indexed_load_store(Instructions::const_iterator begin,
                                                            Instructions::const_iterator end,
                                                            Instructions& out) {
      Instructions::const_iterator it = begin;
      
      /* instruction 1 */
      if (it == end) { return begin; }
      const Instruction *instr1 = *begin++;
   }
   
}
