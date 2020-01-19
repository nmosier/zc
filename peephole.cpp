#include <list>

#include "peephole.hpp"
#include "asm.hpp"

namespace zc::z80 {

   void PeepholeOptimization::ReplaceAll(Instructions& input) const {
      for (const_iterator it = input.begin(), end = input.end();
           it != end;
           ++it) {
         it = ReplaceAt(input, it);
      }
   }

   Instructions::const_iterator PeepholeOptimization::ReplaceAt(Instructions& input,
                                                                const_iterator it) const {
      Instructions new_instrs;
      const_iterator rm_end = replace_(it, input.end(), new_instrs);
      const_iterator new_end = input.erase(it, rm_end);
      input.splice(new_end, new_instrs);
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
      const RegisterValue *rr1;
      const IndexedRegisterValue *idxval;

      /* instruction 1 */
      if (it == end) { return begin; }
      if ((*it)->name() != "lea") { return begin; }
      rr1 = (const RegisterValue *) (*it)->operands()[0];
      idxval = (const IndexedRegisterValue *) (*it)->operands()[1];
      ++it;
      
      /* instruction 2 */
      if (it == end) { return begin; }
      if ((*it)->name() != "ld") { return begin; }
      const MemoryValue *mem_dst = dynamic_cast<const MemoryValue *>((*it)->operands()[0]);
      const MemoryValue *mem_src = dynamic_cast<const MemoryValue *>((*it)->operands()[1]);
      if (mem_dst == nullptr && mem_src == nullptr) { return begin; }

      bool is_store = (mem_dst != nullptr);
      const Value *dst, *src;
      if (is_store) {
         /* store */
         const RegisterValue *reg_dst = dynamic_cast<const RegisterValue *>(mem_dst->loc()->addr());
         if (reg_dst == nullptr || !rr1->Eq(reg_dst)) { return begin; }
         src = (*it)->operands()[1];
      } else {
         /* load */
         const RegisterValue *reg_src = dynamic_cast<const RegisterValue *>(mem_src->loc()->addr());
         if (reg_src == nullptr || !rr1->Eq(reg_src)) { return begin; }
         dst = (*it)->operands()[0];
      }

      const MemoryValue *memval = new MemoryValue(new MemoryLocation(idxval),
                                                  is_store ? (*it)->operands()[1]->size() :
                                                  (*it)->operands()[0]->size());
      if (is_store) {
         dst = memval;
      } else {
         src = memval;
      }

      out.push_back(new LoadInstruction(dst, src));

      return ++it;
   }

   const PeepholeOptimization PH_indexed_load_store(peephole_indexed_load_store);
   
}
