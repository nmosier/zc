/* LEXER REFERENCE: https://www.lysator.liu.se/c/ANSI-C-grammar-l.html */

D  [0-9]          /* digit */
L  [[:alpha:]_]   /* letter */
H  [[:hexdigit:]] /* hexadecimal */
E  [Ee][+-]?{D}+  /* exponent */
               /* OMITTED: FS -- float suffix */
               /* OMITTED: IS -- integer suffix */

%{
#include "ast.h"

#define YY_NO_UNPUT
#define yywrap() 1
#define YY_SKIP_YYWRAP

    extern std::istream *g_istream;

#undef YY_INPUT
#define YY_INPUT(buf, result, max_size)                     \
    g_istream->read((char *) buf, max_size);                \
    if ((result = g_istream->gcount()) < 0) {               \
        YY_FATAL_ERROR("read() in flex scanner failed");    \
    }
};

%}

%%



%%
