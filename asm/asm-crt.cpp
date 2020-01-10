#include <initializer_list>

#include "asm.hpp"

namespace zc::z80 {

   CRT crt;

   CRT::CRT(const std::initializer_list<std::string>& l) {
      for (const std::string& str : l) {
         Label *label = new Label(str);
         LabelValue *val = new LabelValue(label);
         map_[str] = std::pair(label, val);
      }
   }

   const CRT::Pair& CRT::add(const std::string& name) {
      Label *label = new Label(name);
      LabelValue *val = new LabelValue(label);
      return map_[name] = Pair(label, val);
   }
   
   const Label *CRT::label(const std::string& name) {
      auto it = map_.find(name);
      if (it == map_.end()) {
         return add(name).first;
      } else {
         return it->second.first;
      }
   }

   const LabelValue *CRT::val(const std::string& name) {
      auto it = map_.find(name);
      if (it == map_.end()) {
         return add(name).second;
      } else {
         return it->second.second;
      }
   }
   

}
