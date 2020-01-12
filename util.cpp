#ifndef __UTIL_HPP
#define __UTIL_HPP

#include <iostream>
#include <cstring>
#include "ast.hpp"
#include "c.tab.hpp"
#include "util.hpp"

namespace zc {

   const char *token_to_string(int tok) {
      const char *char_tok_strs[] =
         {";", "{", "}", ",", ":", "=", "(", ")", "[", "]", ".", "&", "!", "~",
          "-", "+", "*", "/", "%", "<", ">", "^", "|", "?"
         };
      const char *char_tok_str = ";{},:=()[].&!~-+*/%<>^|?";
      const char *char_tok_ptr;
      off_t char_tok_index;
        
      switch (tok) {
      case 0: return "EOF";
      case ERROR: return "ERROR";
      case EQ: return "EQ";
      case NEQ: return "NEQ";
      case LEQ: return "LEQ";
      case GEQ: return "GEQ";
      case OR: return "OR";
      case AND: return "AND";
      case VOID: return "VOID";
      case CHAR: return "CHAR";
      case SHORT: return "SHORT";
      case INT: return "INT";
      case LONG: return "LONG";
      case STRING: return "STRING";
      case INT_CONST: return "INT_CONST";
      case CHAR_CONST: return "CHAR_CONST";
      case ID: return "ID";
      default:
         if ((char_tok_ptr = strchr(char_tok_str, tok)) == NULL) {
            return "(invalid token)";
         }
         char_tok_index = char_tok_ptr - char_tok_str;
         return char_tok_strs[char_tok_index];
      }
   }
    

   void print_token(std::ostream& out, int token, YYSTYPE yylval) {
      out << token_to_string(token);
      
      switch (token) {
      case STRING:
         out << " \"";
         out << static_cast<StringExpr *>(yylval.expr)->str() << "\"";
         break;
      case INT_CONST:
         out << " " << static_cast<LiteralExpr *>(yylval.expr)->val();
         break;
      case CHAR_CONST:            
         out << " '" << (char) static_cast<LiteralExpr *>(yylval.expr)->val() << "'";
         break;
      case ID:
         out << " " << yylval.identifier->id();
         break;
      case ERROR:
         out << " \"" << yylval.error_msg << "\"";
         break;
      default:
         break;
      }
      out << std::endl;
        
   }

   void dump_token(std::ostream& out, int lineno, int token, YYSTYPE yylval) {
      out << "#" << lineno << " " << token_to_string(token);
      print_token(out, token, yylval);
   }



   
   
}

#endif
