#ifndef __PEEPHOLE_HPP
#define __PEEPHOLE_HPP

#include <list>

#include "asm.hpp"
#include "util.hpp"

namespace zc::z80 {

   class FreeRegister;

   typedef std::list<Instruction *> Instructions;
   
   class PeepholeOptimization {
      using iterator = Instructions::iterator;
      using const_iterator = Instructions::const_iterator;
   public:
      /**
       * Basic peephole optimization function.
       * @param in_begin beginning of current input instruction stream window.
       * @param in_end end of current instruction stream window.
       * @param out output list of replaced instructions (if any).
       * @return end iterator of instructions to delete
       */
      typedef const_iterator (*replace_t)
      (const_iterator in_begin, const_iterator in_end, Instructions& out);
      
      void ReplaceAll() const;
      const_iterator ReplaceAt(const_iterator begin) const;

      PeepholeOptimization(Instructions& input_instrs, replace_t replace):
         input_instrs_(input_instrs), replace_(replace) {}
      
   protected:
      replace_t replace_;
      Instructions& input_instrs_;

      template <class InputIt, class... Ts>
      static std::optional<std::tuple<Ts...>> Cast(InputIt begin, InputIt end) {
         return vec_to_tuple<InputIt, Ts...>(begin, end);
      }
   };
}

#endif
