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

   void PeepholeOptimization::PassBlock(Block *block,
                                        const PeepholeOptimization *optim) {
      Instructions& instrs = block->instrs();
      optim->ReplaceAll(instrs);
   }

   void PeepholeOptimization::Pass(FunctionImpl *impl) const {
      Blocks visited;
      void (*fn)(Block *block, const PeepholeOptimization *optim) =
         PeepholeOptimization::PassBlock;
      impl->entry()->for_each_block(visited, fn, this);
      impl->fin()->for_each_block(visited, fn, this);
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

      const auto no_match = [&](){ out.clear(); return begin; };

      /* unbound values */
      const Register *rr1;
      const Register *rr2;
      int8_t frame_index;
      
      /* instruction 1: lea rr1,ix+* */
      if (it == end) { return no_match(); }
      const RegisterValue rr1_1(&rr1, long_size);
      const IndexedRegisterValue idx_(&rv_ix, &frame_index);
      const LeaInstruction instr1(&rr1_1, &idx_);
      if (!instr1.Match(*it)) { return no_match(); }
      ++it;

      /* instruction 2: pop rr2 */
      if (it == end) { return no_match(); }
      const RegisterValue rr2_2(&rr2, long_size);
      const PopInstruction instr2(&rr2_2);
      if (!instr2.Match(*it)) { return no_match(); }
      out.push_back(*it); /* preserve pop instruction */
      ++it;
      
      /* instruction 3: ld (rr1),rr2 | ld rr2,(rr1) */
      if (it == end) { return no_match(); }
      const RegisterValue rr1_3(rr1);
      const MemoryValue rr1_v_3(&rr1_3, long_size);
      const RegisterValue *rr2_3 = new RegisterValue(rr2);

      const LoadInstruction instr3a(&rr1_v_3, rr2_3);
      const LoadInstruction instr3b(rr2_3, &rr1_v_3);
      const MemoryValue *memval = new MemoryValue
         (new IndexedRegisterValue(&rv_ix, frame_index), long_size);
      const Value *dst, *src;
      if (instr3a.Match(*it)) {
         /* instruction 3a: ld (rr1),rr2 */
         dst = memval;
         src = rr2_3;
      } else if (instr3b.Match(*it)) {
         /* instruction 3b: ld rr2,(rr1) */
         dst = rr2_3;
         src = memval;
      } else {
         return no_match();
      }
      ++it;
      
      /* generate replacement */
      out.push_back(new LoadInstruction(dst, src));
      
      return it;
   }

   Instructions::const_iterator peephole_push_pop(Instructions::const_iterator begin,
                                                  Instructions::const_iterator end,
                                                  Instructions& out) {
      /* push rr1
       * pop rr2
       */
      Instructions::const_iterator it = begin;
      const auto no_match = [&](){ out.clear(); return begin; };

      /* unbound values */
      const Register *rr1;
      const Register *rr2;

      /* instruction 1: push rr1 */
      if (it == end) { return no_match(); }
      const RegisterValue rr1_1(&rr1, long_size);
      const PushInstruction instr1(&rr1_1);
      if (!instr1.Match(*it)) { return no_match(); }
      ++it;

      /* instruction 2: pop rr2 */
      if (it == end) { return no_match(); }
      const RegisterValue rr2_2(&rr2, long_size);
      const PopInstruction instr2(&rr2_2);
      if (!instr2.Match(*it)) { return no_match(); }
      ++it;

      /* if rr1 == rr2, delete matched sequence */
      if (rr1->Eq(rr2)) { return it; }

      /* if rr1 == de and rr2 == hl or vice versa, replace with `ex de,hl` */
      if ((rr1->Eq(&r_de) && rr2->Eq(&r_hl)) ||
          (rr1->Eq(&r_hl) && rr2->Eq(&r_de))) {
         out.push_back(new ExInstruction(&rv_de, &rv_hl));
         return it;
      }

      return no_match();
   }

   Instructions::const_iterator peephole_pea(Instructions::const_iterator begin,
                                             Instructions::const_iterator end,
                                             Instructions& out) {
      /* lea rr,ix+*
       * push rr
       */
      Instructions::const_iterator it = begin;
      const auto no_match = [&](){ out.clear(); return begin; };

      /* unbound values */
      const Register *rr;
      int8_t index;

      /* instruction 1 */
      if (it == end) { return no_match(); }
      const RegisterValue rv_1(&rr, long_size);
      const IndexedRegisterValue ir_1(&rv_ix, &index);
      const LeaInstruction instr1(&rv_1, &ir_1);
      if (!instr1.Match(*it)) { return no_match(); }
      ++it;

      /* instruction 2 */
      if (it == end) { return no_match(); }      
      const RegisterValue rv_2(rr);
      const PushInstruction instr2(&rv_2);
      if (!instr2.Match(*it)) { return no_match(); }
      ++it;

      /* replace */
      out.push_back(new PeaInstruction(new IndexedRegisterValue(&rv_ix, index)));
      return it;
   }

   Instructions::const_iterator peephole_self_load(Instructions::const_iterator begin,
                                                   Instructions::const_iterator end,
                                                   Instructions& out) {
      /* ld r,r */      
      Instructions::const_iterator it = begin;
      const auto no_match = [&](){ out.clear(); return begin; };      

      /* unbound values */
      const Register *r;
      int size;
      
      if (it == end) { return no_match(); }
      const RegisterValue r_1(&r, &size);
      const LoadInstruction instr1(&r_1, &r_1);
      if (!instr1.Match(*it)) { return no_match(); }
      ++it;

      return it;
   }
   


   const std::forward_list<PeepholeOptimization> peephole_optims = 
      {PeepholeOptimization(peephole_indexed_load_store),
       PeepholeOptimization(peephole_push_pop), 
       PeepholeOptimization(peephole_pea),
       PeepholeOptimization(peephole_self_load),
      };

   
   
}
