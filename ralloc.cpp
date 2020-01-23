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

   void VariableRallocInfo::RenameVar() {
      const VariableValue *newval = val->Rename();
      (*gen)->operands()[0] = (*(*gen)->operands()[0])->ReplaceVar(val, newval); /* dst */
      for (auto it : uses) {
         (*it)->operands()[1] = (*(*it)->operands()[1])->ReplaceVar(val, newval); /* src */
      }
      val = newval;
   }

   void RegisterAllocator::ComputeVarLifetimes() {
      int index = 0;
      for (auto instr_it = block_->instrs().begin();
           instr_it != block_->instrs().end();
           ++instr_it) {
         const Instruction *instr = *instr_it;
         const VariableValue *var;

         if (instr->src() && (var = instr->src()->var())) {
            /* variable used */
            auto it = vars_.find(var->id());
            assert(it != vars_.end());
            it->second.uses.push_back(instr_it);
            it->second.interval.end = index;
         }
         
         if (instr->dst() && (var = instr->dst()->var())) {
            /* variable is generated */
            auto it = vars_.find(var->id());
            if (it != vars_.end()) {
               /* variable is generated again; rename previous occurences */
               VariableRallocInfo info = it->second;
               vars_.erase(it);
               info.RenameVar();
               vars_.insert({info.val->id(), info});
            }

            /* add new var */
            vars_.insert({var->id(), VariableRallocInfo(var, instr_it, index)});
         }

         ++index;
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

   void VariableRallocInfo::Dump(std::ostream& os) const {
      os << "var v" << val->id() << std::endl;
      os << "\tinterval:\t";
      interval.Dump(os);
      os << std::endl;
      os << "\tgen:\t";
      (*gen)->Emit(os); 
      os << std::endl;
      for (auto it : uses) {
         os << "\tuse:\t";
         (*it)->Emit(os);
         os << std::endl;
      }
   }

   void RegisterAllocator::Dump(std::ostream& os) const {
      for (auto it : regs_) {
         it.first->Dump(os);
         os << ": ";
         it.second.Dump(os);
      }

      for (auto it : vars_) {
         it.second.Dump(os);
      }
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
