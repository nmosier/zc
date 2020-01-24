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

      void Dump(std::ostream& os) const { os << "[" << begin << "," << end << "]"; }

      RallocInterval(int begin, int end): begin(begin), end(end) {}
      RallocInterval() {}
   };

   typedef std::set<RallocInterval> RallocIntervals;

#if 0
   struct VariableRallocTraits {
      int uses; /*!< number of times this variable is used */
      int bytes; /*!< variable byte size */
      bool is_stack_spillable; /*!< can be spilled onto stack */
      bool requires_reg; /*!< requires register when used */
   };
#endif
   
   /**
    * Variable info for the register allocator.
    */
   struct VariableRallocInfo {
      const VariableValue *val;
      Instructions::iterator gen; /*!< where the variable is assigned (i.e. generated) */
      std::list<Instructions::iterator> uses; /*!< the instructions in which the variable is used */
      RallocInterval interval;

      bool dead() const { return interval.length() == 0; }
      void set_dead() {
         interval.end = interval.begin;
         uses.clear();
      }

      void RenameVar();

      void Dump(std::ostream& os) const;
      
      VariableRallocInfo(const VariableValue *val, Instructions::iterator gen, int begin):
         val(val), gen(gen), interval(begin, begin) {}
      VariableRallocInfo(const VariableValue *val, Instructions::iterator gen,
                         const RallocInterval& interval):
         val(val), gen(gen), interval(interval) {}
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
      void ComputeIntervals();

      void Dump(std::ostream& os) const;
      
      RegisterAllocator(Block *block): block_(block) {}

      static void Ralloc(FunctionImpl& impl);
      static void Ralloc(CgenEnv& env);
      
   protected:
      Block *block_;
      std::unordered_map<int, VariableRallocInfo> vars_;
      std::unordered_map<const ByteRegister *, RallocIntervals> regs_;

      static void RallocBlock(Block *block);
   };
   
}

#endif
