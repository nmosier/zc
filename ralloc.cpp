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

   void VariableRallocInfo::RenameVar() {
      const VariableValue *newval = val->Rename();
      (*gen)->operands()[0] = (*(*gen)->operands()[0])->ReplaceVar(val, newval); /* dst */
      for (auto it : uses) {
         (*it)->operands()[1] = (*(*it)->operands()[1])->ReplaceVar(val, newval); /* src */
      }
      val = newval;
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
                  const VariableValue *var = dynamic_cast<const VariableValue *>(val);
                  
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
         regs_.insert({reg_it.first, RegisterFreeIntervals(reg_free_ints)});
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

   bool RegisterAllocator::TryRegAllocVar(const VariableValue *var) {
      /* get candidate registers */
      std::unordered_set<const Register *> candidate_regs;      
      GetAssignableRegs(var, std::inserter(candidate_regs, candidate_regs.begin()));
      if (candidate_regs.empty()) { return false; }

      /* determine nearby regs */
      VariableRallocInfo& var_info = vars_.at(var->id());
      std::unordered_map<const Register *, int> nearby_regs;     
      
      const RegisterValue *var_src = dynamic_cast<const RegisterValue *>((*var_info.gen)->src());
      if (var_src) {
         ++(nearby_regs[var_src->reg()]);
      }

      for (auto use : var_info.uses) {
         auto var_dst = dynamic_cast<const RegisterValue *>((*use)->dst());
         if (var_dst) {
            ++(nearby_regs[var_dst->reg()]);
         }
      }

      /* remove unfree regs */
      for (auto pair : nearby_regs) {
         if (candidate_regs.find(pair.first) == candidate_regs.end()) {
            nearby_regs.erase(pair.first);
         }
      }

      const Register *reg;
      
      /* find ``nearest'' reg */
      if (nearby_regs.empty()) {
         /* IMPROVE -- might need to pick better */
         reg = *candidate_regs.begin(); // NOTE: guaranteed at this point to be at least one.
      } else {
         reg = std::max_element(nearby_regs.begin(), nearby_regs.end(),
                                [](const auto acc, const auto next) -> bool {
                                   return acc.second > next.second;
                                })->first;
      }
      
      /* allocate reg to var */
      var_info.AssignReg(new RegisterValue(reg));
      
      /* remove register free intervals */
      switch (reg->kind()) {
      case Register::Kind::REG_BYTE:
         regs_.at(dynamic_cast<const ByteRegister *>(reg)).
            remove_interval(var_info.interval);
         break;
         
      case Register::Kind::REG_MULTIBYTE:
         {
            auto multibyte_reg = dynamic_cast<const MultibyteRegister *>(reg);
            for (auto byte_reg : multibyte_reg->regs()) {
               regs_.at(byte_reg).remove_interval(var_info.interval);
            }
         }
         break;
      default: abort();
      }

      return true;
   }
   
   AllocKind RegisterAllocator::AllocateVar(const VariableValue *var) {
      VariableRallocInfo& var_info = vars_.at(var->id());      
      bool has_reg = TryRegAllocVar(var);
      if (has_reg) {
         var_info.alloc_kind = AllocKind::ALLOC_REG;
         return AllocKind::ALLOC_REG;
      }
      if (var_info.requires_reg()) {
         throw std::logic_error("could not allocate register to variable that requires register");
      }

      /* check whether stack-spillable */
      if (var_info.is_stack_spillable()) {
         /* try to stack-spill */
         if (stack_spills_.try_add(var_info.interval)) {
            var_info.StackSpill(block()->instrs());
            return AllocKind::ALLOC_STACK;
         }

      }

      return var_info.alloc_kind = AllocKind::ALLOC_NONE; /* TODO */ 
   }

   bool VariableRallocInfo::requires_reg() const {
      if (val->force_reg()) { return true; }
      if (val->size() == byte_size) { return false; }

      /* check if generated from immediate */
      intmax_t imm;
      const ImmediateValue iv(&imm, val->size());
      const LoadInstruction gen_instr(val, &iv);
      if (gen_instr.Match(*gen)) { return true; }

      /* check if stored to memory value */
      const Value *addr;
      const MemoryValue mv(&addr, val->size());
      const LoadInstruction use_instr(&mv, val);
      if (std::any_of(uses.begin(), uses.end(),
                  [&](auto it) -> bool {
                     return use_instr.Match(*it);
                  })) {
         return true;
      }
      
      return false; // might need to check more things 
   }

   bool VariableRallocInfo::is_stack_spillable() const {
      if (val->size() == byte_size) { return false; } /* must be word/long to be spilled */

      /* verify that gen instruction can be translated into `push' */
      const Register *reg_ptr;
      const RegisterValue reg_val(&reg_ptr, val->size());
      const LoadInstruction gen_instr(val, &reg_val);
      if (!gen_instr.Match(*gen)) { return false; }

      /* verify that each use can be translated into `pop' into register */
      if (!std::all_of(uses.begin(), uses.end(),
                       [&](auto use) -> bool {
                          const Register *reg_ptr;
                          const RegisterValue reg_val(&reg_ptr, val->size());
                          const LoadInstruction use_instr(&reg_val, val);
                          return use_instr.Match(*use);
                       })) {
         return false;
      }
      
      return true;
   }

   void VariableRallocInfo::StackSpill(Instructions& instrs) {
      *gen = new PushInstruction((*gen)->src());
      int i = uses.size();
      for (Instructions::iterator use : uses) {
         auto rv = (*use)->dst();
         *use++ = new PopInstruction(rv);
         if (i > 1) {
            instrs.insert(use, new PushInstruction(rv));
         }
         
         --i;
      }
      
      alloc_kind = AllocKind::ALLOC_STACK;
   }
   
   std::ostream& operator<<(std::ostream& os, AllocKind kind) {
      switch (kind) {
      case AllocKind::ALLOC_NONE: os << "ALLOC_NONE"; return os;
      case AllocKind::ALLOC_REG: os << "ALLOC_REG"; return os;
      case AllocKind::ALLOC_STACK: os << "ALLOC_STACK"; return os;
      case AllocKind::ALLOC_FRAME: os << "ALLOC_FRAME"; return os;
      }
   }

   void RegisterAllocator::RunAllocation() {
      for (auto& var_pair : vars_) {
         AllocateVar(var_pair.second.val);

         var_pair.second.val->Emit(std::cerr);
         std::cerr << ":\t" << var_pair.second.alloc_kind << std::endl;
      }
   }

   RallocIntervals::iterator RegisterFreeIntervals::superinterval(const RallocInterval& interval)
      const {
      auto it = intervals.begin(), end = intervals.end();      
      for (; it != end; ++it) {
         if (interval.in(*it)) {
            break;
         }
      }
      return it;
   }

   void RegisterFreeIntervals::remove_interval(const RallocInterval& interval) {
      auto it = superinterval(interval);
      if (it == intervals.end()) { throw std::logic_error("asked to remove interval not present"); }
      intervals.insert(RallocInterval(it->begin, it->begin_it, interval.begin, interval.begin_it));
      intervals.insert(RallocInterval(interval.end, interval.end_it, it->end, it->end_it));
      intervals.erase(it);
   }

   void VariableRallocInfo::AssignReg(const RegisterValue *reg) {
      (*gen)->ReplaceVar(val, reg);
      for (auto use : uses) {
         (*use)->ReplaceVar(val, reg);
      }
   }


   /*** DUMPS ***/
   void RegisterFreeIntervals::Dump(std::ostream& os) const {
      for (const RallocInterval& interval : intervals) {
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
         for (auto interval : it.second.intervals) {
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

      /* TMP */
      ralloc.RunAllocation();

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

   /*** ***/
   bool NestedRallocIntervals::try_add(const RallocInterval& interval) {
      for (auto nested_interval : intervals_) {
         if (interval.intersects(nested_interval) && !interval.in(nested_interval) &&
             nested_interval.in(interval)) {
            /* overlaps */
            return false;
         }
      }
      
      intervals_.insert(interval);
      return true;
   }
}
