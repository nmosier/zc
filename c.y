/* GRAMMAR REFERENCE: http://www.cs.man.ac.uk/~pjj/bnf/c_syntax.bnf */

%{
  #include "ast.hpp"

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
    zc::StringExpr *string;
    zc::LiteralExpr *literal;
    zc::Identifier *identifier;
    zc::TranslationUnit *translation_unit;
    zc::ExternalDecl *external_decl;
    zc::FunctionDef *function_def;
    zc::Decls *decls;
    zc::Decl *decl;
    zc::DeclSpecs *decl_specs;
    zc::Declarators *declarators;
    zc::Declarator *declarator;
    zc::ParamTypes *param_types;
    zc::ParamType *param_type;
    zc::ParamDecl *param_decl;
    zc::ParamDecls *param_decls;
    zc::ASTStat *stat;
    zc::ASTStats *stats;
    zc::CompoundStat *compound_stat;
    zc::ExprStat *expr_stat;
    zc::ASTExpr *expr;
    zc::AssignentExpr *assignment_expr;
    zc::BinaryExpr *binary_expr;
    zc::UnaryExpr *unary_expr;
    zc::LiteralExpr *literal_expr;
    zc::BinaryExpr::Kind binary_operator;
    zc::UnaryExpr::Kind unary_operator;
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
%token <string> STRING  
%token <literal> INT_CONST
%token <literal> CHAR_CONST
%token <identifier> ID

                         /* IDENTIFIERS */
%type  <translation_unit> translation_unit
%type  <external_decl> external_decl
%type  <function_def>  function_definition
%type  <decls>         optional_decl_list decl_list
%type  <decl>          decl
%type  <decl_specs>    decl_specs
%type  <declarators>   init_declarator_list
%type  <declarator>    init_declarator declarator
%type  <direct_declarator> direct_declarator
%type  <pointer>       optional_pointer pointer
%type  <param_types>   param_types
%type  <param_type>    param_type
%type  <param_decl>    param_decl
%type  <param_decls>   param_type_list param_list
%type  <stat>          stat
%type  <stats>         optional_stat_list stat_list
%type  <compound_stat> compound_stat
%type  <expr_stat>     exp_stat
%type  <expr>          optional_expr exp
%type  <assignment_expr> assignment_exp
%type  <binary_expr>   conditional_exp logical_or_exp logical_and_exp inclusive_or_exp exclusive_or_exp and_exp equality_exp relational_exp shift_expression additive_exp mult_exp
%type  <unary_expr>    unary_exp
%type  <expr>          postfix_exp primary_exp
%type  <literal_expr> constant
%type  <binary_operator> relational_operator additive_operator mult_operator
%type  <unary_operator> unary_operator

                                                
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
assignment_exp: unary_exp '=' conditional_exp
        ; /* incomplete */
conditional_exp:
                logical_or_exp /* TODO */
        ; /* TODO, incomplete */
unary_exp:      postfix_exp /* TODO */
        |       unary_operator unary_exp
        ; /* TODO, incomplete */
logical_or_exp: logical_and_exp
        |       logical_or_exp OR logical_and_exp
        ;
logical_and_exp:
                inclusive_or_exp
        |       logical_and_exp AND inclusive_or_exp
        ;
inclusive_or_exp:
                exclusive_or_exp
        |       inclusive_or_exp '|' exclusive_or_exp
        ;
exclusive_or_exp:
                and_exp
        |       exclusive_or_exp '^' and_exp
        ;
and_exp:        equality_exp
        |       and_exp '&' equality_exp
        ;
equality_exp:   relational_exp
        |       equality_exp EQ relational_exp
        |       equality_exp NEQ relational_exp
        ;
relational_exp: shift_expression
        |       relational_exp relational_operator shift_expression
        ;
shift_expression:
                additive_exp
        ; /* incomplete */
additive_exp:   mult_exp
        |       additive_exp additive_operator mult_exp
        ;
mult_exp:       cast_exp
        |       mult_exp mult_operator cast_exp
        ;
cast_exp:       unary_exp
        ; /* incomplete */

postfix_exp:    primary_exp
        ; /* incomplete */

primary_exp:    ID
        |       constant
        |       '(' exp ')'
        ; /* incomplete */
constant:       INT_CONST
        |       CHAR_CONST
        ; /* incomplete */

relational_operator:
                '<'
        |       LEQ
        ; /* incomplete */
additive_operator:
                '+'
        |       '-'
        ;
mult_operator:  '*'
        |       '/'
        |       '%'
        ;
unary_operator: '&'
        |       '*'
        |       '+'
        |       '-'
        |       '~'
        |       '!'
        ;

%%

