#include <cassert>
#include <list>
#include <unordered_map>
#include <map>

#include "ralloc.hpp"
#include "cgen.hpp"

namespace zc::z80 {

   int RallocInterval::length() const {
      assert(begin >= 0 && end >= begin);
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

#if 0
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
#endif

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

   void RegisterAllocator::ComputeIntervals() {
      enum class mode {GEN, USE};
      std::list<const Value *> gens, uses;
      std::unordered_map<const ByteRegister *,
                         std::map<int,std::pair<Instructions::iterator,mode>>> byte_regs;
      std::unordered_map<const VariableValue *,
                         std::map<int,std::pair<Instructions::iterator,mode>>> vars;

      /* populate local regs maps with all alloc'able regs */
      for (const ByteRegister *reg : {&r_a, &r_b, &r_c, &r_d, &r_e, &r_h, &r_l}) {
         byte_regs[reg];
      }

      /* iterate thru instructions */
      int instr_index = 0;
      for (auto instr_it = block()->instrs().begin();
           instr_it != block()->instrs().end();
           ++instr_it, ++instr_index, gens.clear(), uses.clear()) {
         /* get gens and uses in instruction */
         (*instr_it)->Gen(gens);
         (*instr_it)->Use(uses);

         auto fn =
            [&](mode m)
            {
               for (const Value *val : (m == mode::GEN) ? gens : uses) {
                  const Register *reg = val->reg();
                  const VariableValue *var = val->var();
                  
                  /* add register */
                  if (reg) {
                     /* process byte reg lifetimes */
                     std::list<const ByteRegister *> cur_regs;
                     if (reg->kind() == Register::Kind::REG_MULTIBYTE) {
                        auto arr = dynamic_cast<const MultibyteRegister *>(reg)->regs();
                        cur_regs.insert(cur_regs.end(), arr.begin(), arr.end());
                     } else {
                        cur_regs = {dynamic_cast<const ByteRegister *>(reg)};
                     }
                     for (const ByteRegister *byte_reg : cur_regs) {
                        auto it = byte_regs.find(byte_reg);
                        if (it != byte_regs.end()) {
                           it->second.insert({instr_index, {instr_it, m}});
                        }
                     }
                  }

                  /* add variable */
                  if (var) {
                     vars[var].insert({instr_index, {instr_it, m}});
                  }
               }
            };

         fn(mode::USE);
         fn(mode::GEN);
      }

      /* convert results to intervals */

      /* compute register free intervals, backwards */
      for (auto reg_it : byte_regs) {
         RallocIntervals reg_free_ints;
         auto info_it = reg_it.second.rbegin();
         auto info_end = reg_it.second.rend();
         RallocInterval cur_int;

         /* loop invariant: end set */
         cur_int.end = block()->instrs().size() - 1;
         cur_int.end_it = --block()->instrs().end();
         do {
            /* Find 1st `use'. */
            while (info_it != info_end && info_it->second.second != mode::USE) {
               ++info_it;
            }
            if (info_it == info_end) {
               cur_int.begin = 0;
               cur_int.begin_it = block()->instrs().begin();
            } else {
               cur_int.begin = info_it->first;
               auto tmp_it = info_it->second.first;
               cur_int.begin_it = tmp_it;
            }

            if (cur_int.begin <= cur_int.end) {
               reg_free_ints.insert(cur_int);
            }

            /* skip until `gen' */
            while (info_it != info_end && info_it->second.second != mode::GEN) {
               ++info_it;
            }
            if (info_it != info_end) {
               cur_int.end = info_it->first;
               cur_int.end_it = (Instructions::iterator) info_it->second.first;
            }
         } while (info_it != info_end);

         /* insert into ralloc list */
         regs_.insert({reg_it.first, reg_free_ints});
      }      

      /* process variable lifetime intervals */
      for (auto var_it : vars) {
         auto info_it = var_it.second.begin();
         auto info_end = var_it.second.end();
         assert(info_it->second.second == mode::GEN);

         VariableRallocInfo info(var_it.first, info_it->second.first, info_it->first);

         /* add uses */
         ++info_it;
         while (info_it != info_end) {
            info.uses.push_back(info_it->second.first);
            info.interval.end_it = info_it->second.first;
            info.interval.end = info_it->first;

            ++info_it;
         }

         /* add to vars_ */
         vars_.insert({var_it.first->id(), info});
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
      val->Emit(os);
      os << std::endl;
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
         for (auto interval : it.second) {
            interval.Dump(os);
            os << " ";
         }
         os << std::endl;
      }

      for (auto it : vars_) {
         it.second.Dump(os);
      }
   }

   /*** ***/
   void RegisterAllocator::RallocBlock(Block *block) {
      std::cerr << block->label()->name() << ":" << std::endl;
      RegisterAllocator ralloc(block);
      ralloc.ComputeIntervals();
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
