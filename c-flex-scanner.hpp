#ifndef __C_FLEX_SCANNER_HPP
#define __C_FLEX_SCANNER_HPP

namespace zc {

   class ZC_Scanner: public yyFlexLexer {
   public:

      ZC_Scanner(std::istream *in): yyFlexLexer(in) {
         
      }
      
   private:
      
      SourceLoc loc = 1;
      
   };
   
}

#endif
