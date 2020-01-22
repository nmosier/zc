#ifndef __RALLOC_HPP
#define __RALLOC_HPP

#include <unordered_map>
#include <set>
#include <ostream>

#include "asm.hpp"

namespace zc::z80 {

   /**
    * Interval as used in representing a variable lifetime or when a register is free.
    */
   struct RallocInterval {
      int begin = -1;
      int end = -1;

      int length() const;

      bool operator<(const RallocInterval& other) const {
         return length() < other.length() || begin < other.begin;
      }

      void Dump(std::ostream& os) const { os << "[" << begin << "," << end << "]"; }

      RallocInterval(int begin, int end): begin(begin), end(end) {}
   };

   typedef std::set<RallocInterval> RallocIntervals;

   /**
    * Variable info for the register allocator.
    */
   struct VariableRallocInfo {
      const VariableValue *val;
      const Value **gen; /*!< where the variable is assigned (i.e. generated) */
      const Value **use; /*!< where the variable is used */
      RallocInterval interval;

      VariableRallocInfo(const VariableValue *val, const Value **gen, const Value **use,
                         const RallocInterval& interval):
         val(val), gen(gen), use(use), interval(interval) {}
   };

   /**
    * Register free intervals.
    */
   class RegisterFreeIntervals {
   public:
      RallocIntervals::iterator superinterval(const RallocInterval& interval) const;
      
      void Use(int index);
      void Free(int index);
      void Done();

      void Dump(std::ostream& os) const;

      RegisterFreeIntervals(int instrs_end): instrs_end_(instrs_end), cur_begin_(0) {}
      
   private:
      RallocIntervals intervals_;
      int cur_begin_;
      int instrs_end_;
   };

   /**
    * Block register allocator.
    */
   class RegisterAllocator {
   public:
      Block *block() const { return block_; }

      void ComputeRegIntervals();
      void ComputeVarLifetimes();

      void Dump(std::ostream& os) const;
      
      RegisterAllocator(Block *block): block_(block) {}

      static void Ralloc(FunctionImpl& impl);
      static void Ralloc(CgenEnv& env);
      
   protected:
      Block *block_;
      std::unordered_map<int, VariableRallocInfo> vars_;
      std::unordered_map<const Register *, RegisterFreeIntervals> regs_;

      static void RallocBlock(Block *block);
   };
   
}

#endif
