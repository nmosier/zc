#include <cassert>

#include "ralloc.hpp"
#include "cgen.hpp"

namespace zc::z80 {

   int RallocInterval::length() const {
      assert(begin >= 0 && end > begin);
      return end - begin;
   }

   void RegisterFreeIntervals::Use(int index) {
      if (index > cur_begin_) {
         /* add free interval */
         intervals_.insert(RallocInterval(cur_begin_, index));
      }
      cur_begin_ = instrs_end_; 
   }
   
   void RegisterFreeIntervals::Free(int index) {
      cur_begin_ = index;
   }

   void RegisterFreeIntervals::Done() {
      Use(instrs_end_); /* if was last marked free, then mark as free until end */
   }

   RallocIntervals::iterator RegisterFreeIntervals::superinterval(const RallocInterval& interval)
      const {
      for (auto it = intervals_.lower_bound(interval);
           it != intervals_.end();
           ++it) {
         if (it->begin <= interval.begin && it->end >= interval.end) {
            return it;
         }
      }
      return intervals_.end();
   }

   void RegisterAllocator::ComputeRegIntervals() {
      /* 1. For each instruction in block,
       * 1a.  If destination is register, then mark register as used at this instruction index.
       * 1b.  If source is register, then mark register as free at this instruction index.
       */

      int index = 0;
      for (const Instruction *instr : block_->instrs()) {
         const Register *reg;
         if (instr->src() && (reg = instr->src()->reg())) {
            const Register *reg = instr->src()->reg();
            regs_.try_emplace(reg, block_->instrs().size());
            regs_.at(reg).Free(index);
         }
         if (instr->dst() && (reg = instr->dst()->reg())) {
            regs_.try_emplace(reg, block_->instrs().size());
            regs_.at(reg).Use(index);
         }
         
         ++index;
      }
      
   }

   void RegisterAllocator::ComputeVarLifetimes() {
      struct cmp {
         bool operator()(const VariableValue *a, const VariableValue *b) const {
            return a->Eq(b);
         }
      };
      std::unordered_map<const VariableValue *, VariableRallocInfo,
                         std::hash<const VariableValue *>, cmp> incomplete_vars_;

      int index = 0;
      for (const Instruction *instr : block_->instrs()) {
         const VariableValue *var;
         if (instr->src() && (var = instr->src()->var())) {
            
         }
         if (instr->dst() && (var = instr->dst()->var())) {
            
         }
      }
      
   }


   /*** DUMPS ***/
   void RegisterFreeIntervals::Dump(std::ostream& os) const {
      for (const RallocInterval& interval : intervals_) {
         interval.Dump(os);
         os << ","; 
      }
      os << std::endl;
   }

   void RegisterAllocator::Dump(std::ostream& os) const {
      for (auto it : regs_) {
         it.first->Dump(os);
         os << ": ";
         it.second.Dump(os);
      }
      /* TODO: incomplete */
   }

   /*** ***/
   void RegisterAllocator::RallocBlock(Block *block) {
      std::cerr << block->label()->name() << ":" << std::endl;
      RegisterAllocator ralloc(block);
      ralloc.ComputeRegIntervals();
      ralloc.Dump(std::cerr);
   }

   void RegisterAllocator::Ralloc(FunctionImpl& impl) {
      Blocks visited;
      void (*fn)(Block *) = RegisterAllocator::RallocBlock;
      impl.entry()->for_each_block(visited, fn);
      impl.fin()->for_each_block(visited, fn);
   }

   void RegisterAllocator::Ralloc(CgenEnv& env) {
      for (FunctionImpl& impl : env.impls().impls()) {
         Ralloc(impl);
      }
   }
   
}
