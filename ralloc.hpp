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
         assert(begin <= end && other.begin <= other.end);
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
      const VariableValue *var;
      const Value *allocated_val = nullptr;
      Instructions::iterator gen; /*!< where the variable is assigned (i.e. generated) */
      std::list<Instructions::iterator> uses; /*!< the instructions in which the variable is used */
      RallocInterval interval;
      AllocKind alloc_kind = AllocKind::ALLOC_NONE;

      void AssignVal(const Value *val);
      bool requires_reg() const;

      bool is_stack_spillable() const;
      void StackSpill(Instructions& instrs);

      void FrameSpill(StackFrame& frame); 

      void Dump(std::ostream& os) const;

      VariableRallocInfo(const VariableValue *var, Instructions::iterator gen, int begin):
         var(var), gen(gen), interval(begin, gen, begin, gen) {}

      VariableRallocInfo(const VariableValue *var, Instructions::iterator gen,
                         const RallocInterval& interval):
         var(var), gen(gen), interval(interval) {}

      /**
       * Priority number when scheduling allocations. Higher number means higher priority.
       * Positive values for those that require registers; negative values for those that don't.xs
       */
      double priority() const {
         double base = (uses.size() + 1) / (interval.length() + 1);
         if (requires_reg()) { return base; }
         else { return -1/base; }
      }
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
      
      RegisterAllocator(Block *block, StackFrame& stack_frame):
         block_(block), stack_frame_(stack_frame) {}

      static void Ralloc(FunctionImpl& impl, StackFrame& stack_frame);
      static void Ralloc(CgenEnv& env);
      
   protected:
      Block *block_; /*!< block containing instructions. */
      StackFrame& stack_frame_; /*!< stack frame for spilling locals. */
      typedef std::unordered_map<int, VariableRallocInfo> Vars;
      Vars vars_;
      typedef std::unordered_map<const ByteRegister *, RegisterFreeIntervals> Regs;
      Regs regs_;
      NestedRallocIntervals stack_spills_; /*!< intervals on which vars are stack-spilled */

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
      void GetAssignableRegs(const VariableValue *var,
                             std::unordered_set<const Register *>& out) const;
      
      /**
       * Try to assign register to variable.
       * @param varinfo variable info
       * @return whether a register was assigned
       */
      bool TryAssignReg(VariableRallocInfo& varinfo);
      
      static void RallocBlock(Block *block, StackFrame& stack_frame);
   };
   
}

#endif
