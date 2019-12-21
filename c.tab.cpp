/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ERROR = 258,
     EQ = 259,
     NEQ = 260,
     LEQ = 261,
     GEQ = 262,
     OR = 263,
     AND = 264,
     VOID = 265,
     CHAR = 266,
     SHORT = 267,
     INT = 268,
     LONG = 269,
     STRING = 270,
     INT_CONST = 271,
     CHAR_CONST = 272,
     ID = 273
   };
#endif
/* Tokens.  */
#define ERROR 258
#define EQ 259
#define NEQ 260
#define LEQ 261
#define GEQ 262
#define OR 263
#define AND 264
#define VOID 265
#define CHAR 266
#define SHORT 267
#define INT 268
#define LONG 269
#define STRING 270
#define INT_CONST 271
#define CHAR_CONST 272
#define ID 273




/* Copy the first part of user declarations.  */
#line 3 "c.ypp"


  #include "ast.hpp"

  #define YYDEBUG 1
  #define YYLTYPE zc::SourceLoc
  #define YYLLOC_DEFAULT(Cur, Rhs, N)			\
    (Cur) = (N) ? YYRHSLOC(Rhs, 1) : YYRHSLOC(Rhs, 0);
#define yylloc g_lineno

  void yyerror(const char *);
  int yylex(void);

//  zc::SourceLoc g_lineno = 1;
  zc::TranslationUnit *g_AST_root;

  extern const char *g_filename;
  extern zc::TranslationUnit *g_translation_unit;

  int omerrs = 0; /* Number of lexing/parsing errors */

#include "util.hpp"



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 28 "c.ypp"
{
    const char *error_msg;
    int pointer;
    zc::StringExpr *string;
    zc::LiteralExpr *literal_expr;
    zc::Identifier *identifier;
    zc::TranslationUnit *translation_unit;
    zc::ExternalDecl *external_decl;
    zc::FunctionDef *function_def;
    zc::Decls *decls;
    zc::Decl *decl;
    zc::DeclSpec *decl_spec;
    zc::DeclSpecs *decl_specs;
    zc::ASTDeclarators *declarators;
    zc::ASTDeclarator *declarator;
    zc::Decls *param_types;
    zc::Decl *param_type;
    zc::Decl *param_decl;
    zc::Decls *param_decls;
    zc::ASTStat *stat;
    zc::ASTStats *stats;
    zc::CompoundStat *compound_stat;
    zc::ExprStat *expr_stat;
    zc::ASTExpr *expr;
    zc::AssignmentExpr *assignment_expr;
    zc::BinaryExpr::Kind binary_operator;
    zc::UnaryExpr::Kind unary_operator;
}
/* Line 193 of yacc.c.  */
#line 186 "c.tab.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 211 "c.tab.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  12
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   109

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  37
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  47
/* YYNRULES -- Number of rules.  */
#define YYNRULES  90
/* YYNRULES -- Number of states.  */
#define YYNSTATES  118

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   273

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    36,     2,     2,     2,    34,    29,     2,
      21,    22,    23,    31,    20,    32,     2,    33,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    19,
      30,    26,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    28,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    24,    27,    25,    35,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    12,    16,    17,    19,
      21,    24,    28,    30,    33,    35,    37,    39,    41,    43,
      44,    46,    48,    52,    54,    56,    59,    61,    65,    70,
      72,    75,    77,    79,    83,    86,    88,    90,    91,    93,
      95,    98,   103,   106,   107,   109,   111,   115,   117,   119,
     122,   124,   128,   130,   134,   136,   140,   142,   146,   148,
     152,   154,   158,   160,   164,   166,   168,   172,   174,   178,
     180,   182,   184,   186,   190,   192,   194,   196,   198,   200,
     202,   204,   206,   208,   210,   212,   214,   216,   218,   220,
     222
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      38,     0,    -1,    39,    -1,    38,    39,    -1,    40,    -1,
      43,    -1,    44,    49,    58,    -1,    -1,    42,    -1,    43,
      -1,    42,    43,    -1,    44,    46,    19,    -1,    45,    -1,
      44,    45,    -1,    10,    -1,    11,    -1,    12,    -1,    13,
      -1,    14,    -1,    -1,    47,    -1,    48,    -1,    47,    20,
      48,    -1,    49,    -1,    50,    -1,    51,    50,    -1,    18,
      -1,    21,    49,    22,    -1,    50,    21,    52,    22,    -1,
      23,    -1,    23,    51,    -1,    53,    -1,    54,    -1,    53,
      20,    54,    -1,    44,    49,    -1,    59,    -1,    58,    -1,
      -1,    57,    -1,    55,    -1,    57,    55,    -1,    24,    41,
      56,    25,    -1,    60,    19,    -1,    -1,    61,    -1,    62,
      -1,    64,    26,    63,    -1,    65,    -1,    76,    -1,    83,
      64,    -1,    66,    -1,    65,     8,    66,    -1,    67,    -1,
      66,     9,    67,    -1,    68,    -1,    67,    27,    68,    -1,
      69,    -1,    68,    28,    69,    -1,    70,    -1,    69,    29,
      70,    -1,    71,    -1,    70,    79,    71,    -1,    72,    -1,
      71,    80,    72,    -1,    73,    -1,    74,    -1,    73,    81,
      74,    -1,    75,    -1,    74,    82,    75,    -1,    64,    -1,
      77,    -1,    18,    -1,    78,    -1,    21,    61,    22,    -1,
      16,    -1,    17,    -1,     4,    -1,     5,    -1,    30,    -1,
       6,    -1,    31,    -1,    32,    -1,    23,    -1,    33,    -1,
      34,    -1,    29,    -1,    23,    -1,    31,    -1,    32,    -1,
      35,    -1,    36,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   109,   109,   113,   117,   118,   121,   124,   125,   127,
     128,   130,   137,   138,   140,   141,   142,   143,   144,   147,
     148,   151,   152,   155,   157,   158,   161,   162,   163,   165,
     166,   169,   171,   172,   174,   176,   177,   180,   181,   183,
     184,   186,   190,   192,   193,   195,   197,   200,   202,   203,
     205,   206,   211,   212,   217,   218,   223,   224,   228,   229,
     233,   234,   238,   239,   244,   246,   247,   251,   252,   256,
     259,   262,   263,   264,   266,   267,   271,   272,   275,   276,
     279,   280,   282,   283,   284,   286,   287,   288,   289,   290,
     291
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ERROR", "EQ", "NEQ", "LEQ", "GEQ", "OR",
  "AND", "VOID", "CHAR", "SHORT", "INT", "LONG", "STRING", "INT_CONST",
  "CHAR_CONST", "ID", "';'", "','", "'('", "')'", "'*'", "'{'", "'}'",
  "'='", "'|'", "'^'", "'&'", "'<'", "'+'", "'-'", "'/'", "'%'", "'~'",
  "'!'", "$accept", "translation_unit", "external_decl",
  "function_definition", "optional_decl_list", "decl_list", "decl",
  "decl_specs", "type_spec", "optional_init_declarator_list",
  "init_declarator_list", "init_declarator", "declarator",
  "direct_declarator", "pointer", "param_type_list", "param_list",
  "param_decl", "stat", "optional_stat_list", "stat_list", "compound_stat",
  "exp_stat", "optional_exp", "exp", "assignment_exp", "conditional_exp",
  "unary_exp", "logical_or_exp", "logical_and_exp", "inclusive_or_exp",
  "exclusive_or_exp", "and_exp", "equality_exp", "relational_exp",
  "shift_expression", "additive_exp", "mult_exp", "cast_exp",
  "postfix_exp", "primary_exp", "constant_exp", "equality_operator",
  "relational_operator", "additive_operator", "mult_operator",
  "unary_operator", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,    59,
      44,    40,    41,    42,   123,   125,    61,   124,    94,    38,
      60,    43,    45,    47,    37,   126,    33
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    37,    38,    38,    39,    39,    40,    41,    41,    42,
      42,    43,    44,    44,    45,    45,    45,    45,    45,    46,
      46,    47,    47,    48,    49,    49,    50,    50,    50,    51,
      51,    52,    53,    53,    54,    55,    55,    56,    56,    57,
      57,    58,    59,    60,    60,    61,    62,    63,    64,    64,
      65,    65,    66,    66,    67,    67,    68,    68,    69,    69,
      70,    70,    71,    71,    72,    73,    73,    74,    74,    75,
      76,    77,    77,    77,    78,    78,    79,    79,    80,    80,
      81,    81,    82,    82,    82,    83,    83,    83,    83,    83,
      83
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     3,     0,     1,     1,
       2,     3,     1,     2,     1,     1,     1,     1,     1,     0,
       1,     1,     3,     1,     1,     2,     1,     3,     4,     1,
       2,     1,     1,     3,     2,     1,     1,     0,     1,     1,
       2,     4,     2,     0,     1,     1,     3,     1,     1,     2,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     1,     3,     1,     3,     1,
       1,     1,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    14,    15,    16,    17,    18,     0,     2,     4,     5,
      19,    12,     1,     3,    26,     0,    29,    13,     0,    20,
      21,    23,    24,     0,     0,    30,    11,     0,     7,     6,
       0,    25,    27,    22,    23,    37,     8,     9,    19,     0,
       0,    31,    32,    74,    75,    71,     0,    86,    85,    87,
      88,    89,    90,    39,     0,    38,    36,    35,     0,    44,
      45,     0,    48,    70,    72,     0,    10,    34,    28,     0,
       0,    41,    40,    42,     0,    49,    33,    73,    46,    69,
      47,    50,    52,    54,    56,    58,    60,    62,    64,    65,
      67,     0,     0,     0,     0,     0,    76,    77,     0,    79,
      78,     0,    80,    81,     0,    82,    83,    84,     0,    51,
      53,    55,    57,    59,    61,    63,    66,    68
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     6,     7,     8,    35,    36,     9,    10,    11,    18,
      19,    20,    34,    22,    23,    40,    41,    42,    53,    54,
      55,    56,    57,    58,    59,    60,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    62,
      63,    64,    98,   101,   104,   108,    65
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -35
static const yytype_int8 yypact[] =
{
      54,   -35,   -35,   -35,   -35,   -35,     6,   -35,   -35,   -35,
      67,   -35,   -35,   -35,   -35,     7,   -14,   -35,     3,     9,
     -35,     2,    18,    -7,    22,   -35,   -35,     7,    54,   -35,
      54,    18,   -35,   -35,   -35,    19,    54,   -35,    67,    67,
      25,    29,   -35,   -35,   -35,   -35,    40,   -35,   -35,   -35,
     -35,   -35,   -35,   -35,    34,    19,   -35,   -35,    43,   -35,
     -35,    44,   -35,   -35,   -35,    40,   -35,   -35,   -35,    54,
      38,   -35,   -35,   -35,    40,   -35,   -35,   -35,   -35,   -35,
      65,    73,    47,    55,    57,    41,    -3,   -35,    21,   -10,
     -35,    40,    40,    40,    40,    40,   -35,   -35,    40,   -35,
     -35,    40,   -35,   -35,    40,   -35,   -35,   -35,    40,    73,
      47,    55,    57,    41,    -3,   -35,   -10,   -35
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -35,   -35,    78,   -35,   -35,   -35,   -21,   -28,    -6,   -35,
     -35,    60,    -5,    66,    75,   -35,   -35,    23,    39,   -35,
     -35,    72,   -35,   -35,    49,   -35,   -35,   -34,   -35,     5,
       8,     4,    10,    11,     0,    -2,   -35,    -1,     1,   -35,
     -35,   -35,   -35,   -35,   -35,   -35,   -35
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -44
static const yytype_int8 yytable[] =
{
      38,    61,    39,    99,    17,    21,    12,    37,    38,    16,
      24,    14,    61,   105,    15,    66,     1,     2,     3,     4,
       5,    61,    26,   106,   107,    14,    28,   100,    15,    27,
      16,    75,    17,    17,    67,    43,    44,    45,   -43,    30,
      46,    39,    47,    28,    32,    96,    97,    68,    48,    69,
      49,    50,   102,   103,    51,    52,    43,    44,    45,    71,
      77,    46,    73,    47,     1,     2,     3,     4,     5,    48,
      74,    49,    50,    91,    93,    51,    52,     1,     2,     3,
       4,     5,    92,    94,    13,    14,    95,    33,    15,    31,
      16,    25,    76,    29,    72,    70,   109,   111,   114,   115,
     110,     0,     0,   116,   112,     0,   113,     0,     0,   117
};

static const yytype_int8 yycheck[] =
{
      28,    35,    30,     6,    10,    10,     0,    28,    36,    23,
      15,    18,    46,    23,    21,    36,    10,    11,    12,    13,
      14,    55,    19,    33,    34,    18,    24,    30,    21,    20,
      23,    65,    38,    39,    39,    16,    17,    18,    19,    21,
      21,    69,    23,    24,    22,     4,     5,    22,    29,    20,
      31,    32,    31,    32,    35,    36,    16,    17,    18,    25,
      22,    21,    19,    23,    10,    11,    12,    13,    14,    29,
      26,    31,    32,     8,    27,    35,    36,    10,    11,    12,
      13,    14,     9,    28,     6,    18,    29,    27,    21,    23,
      23,    16,    69,    21,    55,    46,    91,    93,    98,   101,
      92,    -1,    -1,   104,    94,    -1,    95,    -1,    -1,   108
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    10,    11,    12,    13,    14,    38,    39,    40,    43,
      44,    45,     0,    39,    18,    21,    23,    45,    46,    47,
      48,    49,    50,    51,    49,    51,    19,    20,    24,    58,
      21,    50,    22,    48,    49,    41,    42,    43,    44,    44,
      52,    53,    54,    16,    17,    18,    21,    23,    29,    31,
      32,    35,    36,    55,    56,    57,    58,    59,    60,    61,
      62,    64,    76,    77,    78,    83,    43,    49,    22,    20,
      61,    25,    55,    19,    26,    64,    54,    22,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,     8,     9,    27,    28,    29,     4,     5,    79,     6,
      30,    80,    31,    32,    81,    23,    33,    34,    82,    66,
      67,    68,    69,    70,    71,    72,    74,    75
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;
/* Location data for the look-ahead symbol.  */
YYLTYPE yylloc;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[2];

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;
#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 0;
#endif


  /* User initialization code.  */
#line 57 "c.ypp"
{
    yylloc = 1;
}
/* Line 1078 of yacc.c.  */
#line 1376 "c.tab.cpp"
  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
	YYSTACK_RELOCATE (yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 109 "c.ypp"
    {
                    g_AST_root = (yyval.translation_unit) = zc::TranslationUnit::Create(zc::ExternalDecls::Create((yylsp[(1) - (1)])), (yylsp[(1) - (1)]));
                    (yyval.translation_unit)->decls()->vec().push_back((yyvsp[(1) - (1)].external_decl));
                ;}
    break;

  case 3:
#line 113 "c.ypp"
    {
                    (yyvsp[(1) - (2)].translation_unit)->decls()->vec().push_back((yyvsp[(2) - (2)].external_decl)); (yyval.translation_unit) = (yyvsp[(1) - (2)].translation_unit); (yyloc) = (yylsp[(1) - (2)]);
                ;}
    break;

  case 4:
#line 117 "c.ypp"
    { (yyval.external_decl) = (yyvsp[(1) - (1)].function_def); ;}
    break;

  case 5:
#line 118 "c.ypp"
    { (yyval.external_decl) = (yyvsp[(1) - (1)].decls); ;}
    break;

  case 6:
#line 121 "c.ypp"
    { (yyval.function_def) = zc::FunctionDef::Create((yyvsp[(1) - (3)].decl_specs), (yyvsp[(2) - (3)].declarator), (yyvsp[(3) - (3)].compound_stat), (yylsp[(1) - (3)])); ;}
    break;

  case 7:
#line 124 "c.ypp"
    { (yyval.decls) = zc::Decls::Create((yyloc)); ;}
    break;

  case 8:
#line 125 "c.ypp"
    { (yyval.decls) = (yyvsp[(1) - (1)].decls); ;}
    break;

  case 10:
#line 128 "c.ypp"
    { (yyvsp[(1) - (2)].decls)->vec().insert((yyvsp[(1) - (2)].decls)->vec().end(), (yyvsp[(2) - (2)].decls)->vec().begin(), (yyvsp[(2) - (2)].decls)->vec().end()); ;}
    break;

  case 11:
#line 130 "c.ypp"
    {
                    (yyval.decls) = zc::Decls::Create((yylsp[(1) - (3)]));
                    for (auto decl : (yyvsp[(2) - (3)].declarators)->vec()) {
                        (yyval.decls)->vec().push_back(zc::Decl::Create((yyvsp[(1) - (3)].decl_specs), decl, (yylsp[(1) - (3)])));
                    }
                ;}
    break;

  case 12:
#line 137 "c.ypp"
    { (yyval.decl_specs) = zc::DeclSpecs::Create((yylsp[(1) - (1)])); (yyval.decl_specs)->vec().push_back((yyvsp[(1) - (1)].decl_spec)); ;}
    break;

  case 13:
#line 138 "c.ypp"
    { (yyval.decl_specs) = (yyvsp[(1) - (2)].decl_specs); (yyval.decl_specs)->vec().push_back((yyvsp[(2) - (2)].decl_spec)); (yyloc) = (yylsp[(1) - (2)]); ;}
    break;

  case 14:
#line 140 "c.ypp"
    { (yyval.decl_spec) = zc::TypeSpec::Create(zc::TypeSpec::TYPE_VOID, (yylsp[(1) - (1)])); ;}
    break;

  case 15:
#line 141 "c.ypp"
    { (yyval.decl_spec) = zc::TypeSpec::Create(zc::TypeSpec::TYPE_CHAR, (yylsp[(1) - (1)])); ;}
    break;

  case 16:
#line 142 "c.ypp"
    { (yyval.decl_spec) = zc::TypeSpec::Create(zc::TypeSpec::TYPE_SHORT, (yylsp[(1) - (1)])); ;}
    break;

  case 17:
#line 143 "c.ypp"
    { (yyval.decl_spec) = zc::TypeSpec::Create(zc::TypeSpec::TYPE_INT, (yylsp[(1) - (1)])); ;}
    break;

  case 18:
#line 144 "c.ypp"
    { (yyval.decl_spec) = zc::TypeSpec::Create(zc::TypeSpec::TYPE_LONG, (yylsp[(1) - (1)])); ;}
    break;

  case 19:
#line 147 "c.ypp"
    { (yyval.declarators) = zc::ASTDeclarators::Create((yyloc)); (yyloc) = 0; ;}
    break;

  case 21:
#line 151 "c.ypp"
    { (yyval.declarators) = zc::ASTDeclarators::Create((yylsp[(1) - (1)])); (yyval.declarators)->vec().push_back((yyvsp[(1) - (1)].declarator)); ;}
    break;

  case 22:
#line 152 "c.ypp"
    { (yyval.declarators) = (yyvsp[(1) - (3)].declarators); (yyval.declarators)->vec().push_back((yyvsp[(3) - (3)].declarator)); ;}
    break;

  case 24:
#line 157 "c.ypp"
    { (yyval.declarator) = (yyvsp[(1) - (1)].declarator); ;}
    break;

  case 25:
#line 158 "c.ypp"
    { (yyval.declarator) = zc::PointerDeclarator::Create((yyvsp[(1) - (2)].pointer), (yyvsp[(2) - (2)].declarator), (yylsp[(1) - (2)])); ;}
    break;

  case 26:
#line 161 "c.ypp"
    { (yyval.declarator) = zc::BasicDeclarator::Create((yyvsp[(1) - (1)].identifier), (yylsp[(1) - (1)])); ;}
    break;

  case 27:
#line 162 "c.ypp"
    { (yyval.declarator) = (yyvsp[(2) - (3)].declarator); ;}
    break;

  case 28:
#line 163 "c.ypp"
    { (yyval.declarator) = zc::FunctionDeclarator::Create((yyvsp[(1) - (4)].declarator), (yyvsp[(3) - (4)].param_decls), (yylsp[(1) - (4)])); ;}
    break;

  case 29:
#line 165 "c.ypp"
    { (yyval.pointer) = 1; ;}
    break;

  case 30:
#line 166 "c.ypp"
    { (yyval.pointer) = (yyvsp[(2) - (2)].pointer) + 1; ;}
    break;

  case 32:
#line 171 "c.ypp"
    { (yyval.param_decls) = zc::Decls::Create((yylsp[(1) - (1)])); (yyval.param_decls)->vec().push_back((yyvsp[(1) - (1)].param_decl)); ;}
    break;

  case 33:
#line 172 "c.ypp"
    { (yyval.param_decls) = (yyvsp[(1) - (3)].param_decls); (yyval.param_decls)->vec().push_back((yyvsp[(3) - (3)].param_decl)); ;}
    break;

  case 34:
#line 174 "c.ypp"
    { (yyval.param_decl) = zc::Decl::Create((yyvsp[(1) - (2)].decl_specs), (yyvsp[(2) - (2)].declarator), (yylsp[(1) - (2)])); ;}
    break;

  case 35:
#line 176 "c.ypp"
    { (yyval.stat) = (yyvsp[(1) - (1)].expr_stat); ;}
    break;

  case 36:
#line 177 "c.ypp"
    { (yyval.stat) = (yyvsp[(1) - (1)].compound_stat); ;}
    break;

  case 37:
#line 180 "c.ypp"
    { (yyval.stats) = zc::ASTStats::Create((yyloc)); (yyloc) = 0; ;}
    break;

  case 38:
#line 181 "c.ypp"
    { (yyval.stats) = (yyvsp[(1) - (1)].stats); ;}
    break;

  case 39:
#line 183 "c.ypp"
    { (yyval.stats) = zc::ASTStats::Create((yylsp[(1) - (1)])); (yyval.stats)->vec().push_back((yyvsp[(1) - (1)].stat)); ;}
    break;

  case 40:
#line 184 "c.ypp"
    { (yyval.stats)->vec().push_back((yyvsp[(2) - (2)].stat)); ;}
    break;

  case 41:
#line 186 "c.ypp"
    {
                    (yyval.compound_stat) = zc::CompoundStat::Create((yyvsp[(2) - (4)].decls), (yyvsp[(3) - (4)].stats), (yylsp[(1) - (4)]));
                ;}
    break;

  case 42:
#line 190 "c.ypp"
    { (yyval.expr_stat) = zc::ExprStat::Create((yyvsp[(1) - (2)].expr), (yylsp[(1) - (2)])); ;}
    break;

  case 43:
#line 192 "c.ypp"
    { (yyval.expr) = zc::NoExpr::Create((yyloc)); (yyloc) = 0; ;}
    break;

  case 45:
#line 195 "c.ypp"
    { (yyval.expr) = (yyvsp[(1) - (1)].assignment_expr); ;}
    break;

  case 46:
#line 197 "c.ypp"
    { (yyval.assignment_expr) = zc::AssignmentExpr::Create((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), (yylsp[(1) - (3)])); ;}
    break;

  case 49:
#line 203 "c.ypp"
    { (yyval.expr) = zc::UnaryExpr::Create((yyvsp[(1) - (2)].unary_operator), (yyvsp[(2) - (2)].expr), (yylsp[(1) - (2)])); ;}
    break;

  case 51:
#line 206 "c.ypp"
    {
                    (yyval.expr) = zc::BinaryExpr::Create(zc::BinaryExpr::BOP_LOGICAL_OR, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), (yylsp[(1) - (3)]));
                ;}
    break;

  case 53:
#line 212 "c.ypp"
    {
                    (yyval.expr) = zc::BinaryExpr::Create(zc::BinaryExpr::BOP_LOGICAL_AND, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), (yylsp[(1) - (3)]));
                ;}
    break;

  case 55:
#line 218 "c.ypp"
    {
                    (yyval.expr) = zc::BinaryExpr::Create(zc::BinaryExpr::BOP_BITWISE_OR, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), (yylsp[(1) - (3)]));
                ;}
    break;

  case 57:
#line 224 "c.ypp"
    {
                    (yyval.expr) = zc::BinaryExpr::Create(zc::BinaryExpr::BOP_BITWISE_XOR, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), (yylsp[(1) - (3)]));
                ;}
    break;

  case 59:
#line 229 "c.ypp"
    {
                    (yyval.expr) = zc::BinaryExpr::Create(zc::BinaryExpr::BOP_BITWISE_AND, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), (yylsp[(1) - (3)]));
                ;}
    break;

  case 61:
#line 234 "c.ypp"
    {
                    (yyval.expr) = zc::BinaryExpr::Create((yyvsp[(2) - (3)].binary_operator), (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), (yylsp[(1) - (3)]));
                ;}
    break;

  case 63:
#line 239 "c.ypp"
    {
                    (yyval.expr) = zc::BinaryExpr::Create((yyvsp[(2) - (3)].binary_operator), (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), (yylsp[(1) - (3)]));
                ;}
    break;

  case 66:
#line 247 "c.ypp"
    {
                    (yyval.expr) = zc::BinaryExpr::Create((yyvsp[(2) - (3)].binary_operator), (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), (yylsp[(1) - (3)]));
                ;}
    break;

  case 68:
#line 252 "c.ypp"
    {
                    (yyval.expr) = zc::BinaryExpr::Create((yyvsp[(2) - (3)].binary_operator), (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), (yylsp[(1) - (3)]));
                ;}
    break;

  case 71:
#line 262 "c.ypp"
    { (yyval.expr) = zc::IdentifierExpr::Create((yyvsp[(1) - (1)].identifier), (yylsp[(1) - (1)])); ;}
    break;

  case 73:
#line 264 "c.ypp"
    { (yyval.expr) = (yyvsp[(2) - (3)].expr); ;}
    break;

  case 76:
#line 271 "c.ypp"
    { (yyval.binary_operator) = zc::BinaryExpr::Kind::BOP_EQ; ;}
    break;

  case 77:
#line 272 "c.ypp"
    { (yyval.binary_operator) = zc::BinaryExpr::Kind::BOP_NEQ; ;}
    break;

  case 78:
#line 275 "c.ypp"
    { (yyval.binary_operator) = zc::BinaryExpr::Kind::BOP_LT; ;}
    break;

  case 79:
#line 276 "c.ypp"
    { (yyval.binary_operator) = zc::BinaryExpr::Kind::BOP_LEQ; ;}
    break;

  case 80:
#line 279 "c.ypp"
    { (yyval.binary_operator) = zc::BinaryExpr::Kind::BOP_PLUS; ;}
    break;

  case 81:
#line 280 "c.ypp"
    { (yyval.binary_operator) = zc::BinaryExpr::Kind::BOP_MINUS; ;}
    break;

  case 82:
#line 282 "c.ypp"
    { (yyval.binary_operator) = zc::BinaryExpr::Kind::BOP_TIMES; ;}
    break;

  case 83:
#line 283 "c.ypp"
    { (yyval.binary_operator) = zc::BinaryExpr::Kind::BOP_DIVIDE; ;}
    break;

  case 84:
#line 284 "c.ypp"
    { (yyval.binary_operator) = zc::BinaryExpr::Kind::BOP_MOD; ;}
    break;

  case 85:
#line 286 "c.ypp"
    { (yyval.unary_operator) = zc::UnaryExpr::Kind::UOP_ADDR; ;}
    break;

  case 86:
#line 287 "c.ypp"
    { (yyval.unary_operator) = zc::UnaryExpr::Kind::UOP_DEREFERENCE; ;}
    break;

  case 87:
#line 288 "c.ypp"
    { (yyval.unary_operator) = zc::UnaryExpr::Kind::UOP_POSITIVE; ;}
    break;

  case 88:
#line 289 "c.ypp"
    { (yyval.unary_operator) = zc::UnaryExpr::Kind::UOP_NEGATIVE; ;}
    break;

  case 89:
#line 290 "c.ypp"
    { (yyval.unary_operator) = zc::UnaryExpr::Kind::UOP_BITWISE_NOT; ;}
    break;

  case 90:
#line 291 "c.ypp"
    { (yyval.unary_operator) = zc::UnaryExpr::Kind::UOP_LOGICAL_NOT; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 1927 "c.tab.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 294 "c.ypp"


void yyerror(const char *s) {
    extern zc::SourceLoc g_lineno;

    std::cerr << "\"" << g_filename << "\", " << "line " << g_lineno << ": " << s << " at or near ";
    zc::print_token(std::cerr, yychar, yylval);
    std::cerr << std::endl;

    ++omerrs;

    if (omerrs > 50) {
        std::cerr << "More than 50 errors" << std::endl;
        exit(1);
    }
}

