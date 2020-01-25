#ifndef __RALLOC_HPP
#define __RALLOC_HPP

#include <unordered_map>
#include <set>
#include <list>
#include <ostream>

#include "asm.hpp"

namespace zc::z80 {

   /**
    * Interval as used in representing a variable lifetime or when a register is free.
    */
   struct RallocInterval {
      int begin = -1;
      Instructions::iterator begin_it;
      int end = -1;
      Instructions::iterator end_it;

      int length() const;

      bool operator<(const RallocInterval& other) const {
         return length() < other.length() || begin < other.begin;
      }

      bool in(const RallocInterval& other) const {
         return begin >= other.begin && end <= other.end;
      }

      bool intersects(const RallocInterval& other) const {
         return (begin >= other.begin && begin <= other.end) ||
            (end >= other.begin && end <= other.end);
      }

      void Dump(std::ostream& os) const { os << "[" << begin << "," << end << "]"; }

      RallocInterval(int begin, Instructions::iterator begin_it,
                     int end, Instructions::iterator end_it):
         begin(begin), begin_it(begin_it), end(end), end_it(end_it) {}
      RallocInterval() {}
   };

   typedef std::set<RallocInterval> RallocIntervals;

   class NestedRallocIntervals {
   public:
      bool try_add(const RallocInterval& interval);
      
   protected:
      RallocIntervals intervals_;
   };

   enum class AllocKind {ALLOC_NONE, ALLOC_REG, ALLOC_STACK, ALLOC_FRAME};
   std::ostream& operator<<(std::ostream& os, AllocKind kind);
   
   /**
    * Variable info for the register allocator.
    */
   struct VariableRallocInfo {
      const VariableValue *val;
      Instructions::iterator gen; /*!< where the variable is assigned (i.e. generated) */
      std::list<Instructions::iterator> uses; /*!< the instructions in which the variable is used */
      RallocInterval interval;
      AllocKind alloc_kind;

      void RenameVar();
      void AssignReg(const RegisterValue *reg);

      bool is_stack_spillable() const;
      void StackSpill(Instructions& instrs);

      bool requires_reg() const;

      void Dump(std::ostream& os) const;

      VariableRallocInfo(const VariableValue *val, Instructions::iterator gen, int begin):
         val(val), gen(gen), interval(begin, gen, begin, gen) {}

      VariableRallocInfo(const VariableValue *val, Instructions::iterator gen,
                         const RallocInterval& interval):
         val(val), gen(gen), interval(interval) {}
   };

   /**
    * Register free intervals.
    */
   struct RegisterFreeIntervals {
      RallocIntervals intervals;

      RallocIntervals::iterator superinterval(const RallocInterval& subinterval) const;
      
      void remove_interval(const RallocInterval& interval);
      void Dump(std::ostream& os) const;

      RegisterFreeIntervals(const RallocIntervals& intervals): intervals(intervals) {}
   };

   /**
    * Block register allocator.
    */
   class RegisterAllocator {
   public:
      Block *block() const { return block_; }

      void ComputeIntervals();
      void RunAllocation();

      void Dump(std::ostream& os) const;
      
      RegisterAllocator(Block *block): block_(block) {}

      static void Ralloc(FunctionImpl& impl);
      static void Ralloc(CgenEnv& env);
      
   protected:
      Block *block_;
      std::unordered_map<int, VariableRallocInfo> vars_;
      std::unordered_map<const ByteRegister *, RegisterFreeIntervals> regs_;
      NestedRallocIntervals stack_spills_;

      AllocKind AllocateVar(const VariableValue *var);


      /**
       * Try to allocate register to variable.
       * @param var variable to allocate to
       * @return whether allocation was successful
       */
      bool TryRegAllocVar(const VariableValue *var); 
      
      /**
       * Get registers that can be assigned to given variable.
       * @tparam output iterator to byte register
       * @param var variable
       * @param out output list of registers
       */
      template <typename OutputIt>
      void GetAssignableRegs(const VariableValue *var, OutputIt out) const {
         /* get variable lifetime */
         const VariableRallocInfo& varinfo = vars_.at(var->id());
         const RallocInterval& varint = varinfo.interval;
         
         /* look thru byte regs to find free */
         std::unordered_set<const ByteRegister *> byte_regs_;
         for (auto reg_it : regs_) {
            auto super_it = reg_it.second.superinterval(varint);
            if (super_it != reg_it.second.intervals.end()) {
               byte_regs_.insert(reg_it.first);
            }
         }

         /* if multibyte var, find register pairs in available byte regs */
         switch (var->size()) {
         case byte_size:
            for (auto byte_reg : byte_regs_) {
               *out++ = byte_reg;
            }
            break;
            
         case long_size:
            for (const MultibyteRegister *multibyte_reg : {&r_bc, &r_de, &r_hl}) {
               auto subregs = multibyte_reg->regs();
               if (std::all_of(subregs.begin(), subregs.end(),
                           [&](const ByteRegister *byte_reg) -> bool {
                              return byte_regs_.find(byte_reg) != byte_regs_.end();
                           })) {
                  *out++ = multibyte_reg;
               }
            }
            break;
            
         default: abort();
         }
      }
      
      /**
       * Try to assign register to variable.
       * @param varinfo variable info
       * @return whether a register was assigned
       */
      bool TryAssignReg(VariableRallocInfo& varinfo);
      
      static void RallocBlock(Block *block);
   };
   
}

#endif
