/* GRAMMAR REFERENCE: http://www.cs.man.ac.uk/~pjj/bnf/c_syntax.bnf */

%{
  #include "ast.h"

  #define YYDEBUG 1
  #define YYLTYPE zc::SourceLoc
  #define YYLLOC_DEFAULT(Cur, Rhs, N)			\
    (Cur) = (N) ? YYRHSLOC(Rhs, 1) : YYRHSLOC(Rhs, 0);
  void yyerror(const char *);
  int yylex(void);
  extern SourceLoc g_lineno;
  extern const char *g_filename;
  extern TranslationUnit *g_translation_unit;


%}
                        
%union {
    char *error_msg;
    zc::TranslationUnit *translation_unit;
    zc::ExternalDecl *external_decl;
    zc::Decl *decl;
    zc::Decls *decls;
    zc::DeclSpecs *decl_specs;
    zc::Declarators *declarators;
    zc::TypeSpec *type_spec;
    zc::FunctionDef *function_def;
    zc::DirectDeclarator *direct_declarator;
    zc::Pointer *pointer;
    zc::ParamTypeList *param_type_list;
    zc::Stat *stat;
    zc::Stats *stats;
    zc::CompoundStat *compound_stat;
    zc::ExprStat *expr_stat;
    zc::UnaryOperator *unary_operator;
    zc::CastExpr *cast_expr;
    zc::UnaryCastExpr *unary_cast_expr;
    zc::Expr *expr;
}

/* terminals */
%token ERROR
                        
/* CONTROL FLOW TERMINALS */
%token WHILE
%token IF

 /* OPERATORS */
 /* -- UNARY OPERATORS */
%token INC      /* '++' */
%token DEC      /* '--' */

                         /* -- BINARY OPERATORS */
%token EQ       /* '==' */
%token NEQ      /* '!=' */
%token LTE      /* '<=' */
%token GTE      /* '>=' */
%token OR   /* '||' */
%token AND  /* '&&' */
%token LSHIFT   /* '<<' */
%token RSHIFT   /* '>>' */

 /* TYPES */
%token CHAR
%token SHORT            
%token INT
%token LONG

                         /* IDENTIFIERS */
%token <string> STRING  
%token <literal> INT_CONST
%token <literal> CHAR_CONST
%token  <symbol>        ID

%type   <translation_unit> translation_unit
%type   <external_decl> external_decl
%type   <decl>          decl
%type   <decl_specs>    decl_specs
%type   <declarators>         init_declarator_list
%type   <declarator>    init_declarator
%type   <declarator>    declarator
%type   <type_spec>     type_spec
%type   <function_def>   function_definition
%type   <direct_declarator> direct_declarator
%type   <pointer>       optional_pointer
%type   <pointer>       pointer
%type   <param_type_list> param_type_list
%type   <param_type_list> param_list
%type   <param_decl>    param_decl
%type   <decls>         decl_list
%type   <decls>         optional_decl_list
%type   <stat>          stat
%type   <compound_stat> compound_stat
%type   <stats>         optional_stat_list
%type   <stats>         stat_list
%type   <expr_stat>     exp_stat
%type   <unary_operator> unary_operator
%type   <cast_expr>          cast_exp
%type   <unary_cast_expr> unary_cast_exp
%type   <expr>          exp
%%

translation_unit:
                external_decl { $$ = TranslationUnit::Create() }
        |       translation_unit external_decl
        ;
external_decl:  function_definition
        |       decl
        ;
function_definition:
                decl_specs declarator compound_stat /* simplified from reference */
        ;
optional_decl_list:
                /* empty */
        |       decl_list
        ;
decl_list:      decl
        |       decl_list decl
        ;
decl:           decl_specs init_declarator_list ';'
        |       decl_specs ';'
        ;
decl_specs:     decl_specs declarator
        ; /* incomplete */
init_declarator_list:
                init_declarator
        |       init_declarator_list ',' init_declarator
        ; /* incomplete */
init_declarator:
                declarator
        ;
declarator:     optional_pointer direct_declarator /* modifications made from original */
        |       optional_pointer '(' declarator ')'
        ;
direct_declarator:
                ID
        |       '(' declarator ')'
        |       direct_declarator '(' param_type_list ')'
        ; /* incomplete */
optional_pointer:
                /* empty */
        |       pointer
        ;
pointer:        '*'
        |       '*' pointer
        ;
param_type_list:
                param_list
        ; /* incomplete */
param_list:     param_decl
        |       param_list ',' param_decl
        ;
param_decl:     decl_specs declarator
        ; /* incomplete */
stat:           exp_stat /* TODO */
        |       compound_stat
        |       selection_stat /* TODO */
        |       iteration_stat /* TODO */
        |       jump_stat /* TODO */
        ; /* TODO, incomplete */
optional_stat_list:
                /* empty */
        |       stat_list
        ;
stat_list:      stat
        |       stat_list stat
        ;
compound_stat:  '{' optional_decl_list optional_stat_list '}'
        ;
exp_stat:       optional_exp ';'
        ;
optional_exp:   /* empty */
        |       exp
        ; /* TODO */
exp:            assignment_exp
        ; /* TODO, incomplete */
assignment_exp: conditional_exp
        |       unary_exp '=' assignment_exp
        ; /* TODO */
conditional_exp:
                logical_or_exp /* TODO */
        ; /* TODO, incomplete */
unary_exp:      postfix_exp /* TODO */
        |       unary_cast_exp
        ; /* TODO, incomplete */
unary_cast_exp: unary_operator cast_exp
        ;
unary_operator: '&'
        |       '*'
        |       '+'
        |       '-'
        |       '~'
        |       '!'
        ;
cast_exp:       unary_exp
        |       '(' type_name ')' unary_exp /* TODO */
        ; /* TODO, modified from reference to allow up to one typename */

%%

