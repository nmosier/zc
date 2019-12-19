%{
 /* Adapted from http://www.cs.man.ac.uk/~pjj/bnf/c_syntax.bnf */

#include "ast.h"

%}

%union {
    char *id;
    char *string; 
    struct translation_unit translation_unit;
    struct external_decl external_decl;
    struct function_definition function_definition;
    struct decl decl;
    struct decl_list decl_list;
    struct decl_specs decl_specs;
    struct type_spec type_spec;
    struct struct_or_union_spec struct_or_union_spec;
    struct struct_or_union struct_or_union;
    struct struct_decl_list struct_decl_list;
    struct init_declarator_list init_declarator_list;
    struct init_declarator init_declarator;
    struct struct_decl struct_decl;
    struct spec_qualifier_list spec_qualifier_list;
    struct struct_declarator_list struct_declarator_list;
    struct struct_declarator struct_declarator;
    struct declarator declarator;
    struct direct_declarator direct_declarator;
    struct pointer pointer;
    struct param_type_list param_type_list;
    struct param_list param_list;
    struct param_decl param_decl;
    struct id_list id_list;
    struct initializer initializer;
    struct initalizer_list initializer_list;
    struct type_name type_name;
    struct abstract_declarator abstract_declarator;
    struct direct_abstract_declarator direct_abstract_declarator;
    struct stat stat;
    struct exp_stat exp_stat;
    struct compound_stat compound_stat;
    struct stat_list stat_list;
    struct selection_stat selection_stat;
    struct iteration_stat iteration_stat;
    struct jump_stat jump_stat;
    struct exp exp;
    struct assignment_exp assignment_exp;
    struct conditional_exp conditional_exp;
    struct const_exp const_exp;
    struct logical_or_exp logical_or_exp;
    struct logical_and_exp logical_and_exp;
    struct inclusive_or_exp inclusive_or_exp;
    struct exclusive_or_exp exclusive_or_exp;
    struct and_exp and_exp;
    struct equality_exp equality_exp;
    struct relational_exp relational_exp;
    struct shift_expression shift_expression;
    struct additive_exp additive_exp;
    struct mult_exp mult_exp;
    struct cast_exp cast_exp;
    struct unary_exp unary_exp;
    struct unary_operator unary_operator;
    struct postfix_exp postfix_exp;
    struct primary_exp primary_exp;
    struct literal literal;
}

/* UNTAGGED TOKENS */

/* TYPE SPECS */        
%token VOID
%token CHAR
%token SHORT
%token INT
%token LONG
%token SIGNED
%token UNSIGNED
%token STRUCT
%token UNION
%token LDOTS
%token SIZEOF
%token INC
%token DEC
%token ARROW
%token AND
%token OR
%token EQ
%token NEQ
%token LEQ
%token GEQ
%token LSHIFT
%token RSHIFT
%token WHILE
%token CONTINUE
%token BREAK
%token RETURN
%token GOTO
%token IF
%token ELSE
                                                
                        /* TAGGED TOKENS */
%token  INT_CONST
%token  CHAR_CONST
%token  <id> ID
%token  <string> STRING

                        /* NON-TERMINALS */
%type   <translation_unit> translation_unit
%type   <external_decl>  external_decl
%type   <function_definition> function_definition
%type   <decl>          decl
%type   <decl_list>     decl_list
%type   <decl_specs>    decl_specs
%type   <type_spec>     type_spec
%type   <struct_or_union_spec> struct_or_union_spec
%type   <struct_or_union>  struct_or_union
%type   <struct_decl_list> struct_decl_list
%type   <init_declarator_list> init_declarator_list
%type   <init_declarator> init_declarator
%type   <struct_decl>   struct_decl
%type   <spec_qualifier_list> spec_qualifier_list
%type   <struct_declarator_list> struct_declarator_list
%type   <struct_declarator> struct_declarator
%type   <declarator>    declarator
%type   <direct_declarator> direct_declarator
%type   <pointer>       pointer
%type   <param_type_list> param_type_list
%type   <param_list>    param_list
%type   <param_decl>    param_decl
%type   <id_list>       id_list
%type   <initializer>   initializer
%type   <initalizer_list> initializer_list
%type   <type_name>     type_name
%type   <abstract_declarator> abstract_declarator
%type   <direct_abstract_declarator> direct_abstract_declarator
%type   <stat>          stat
%type   <exp_stat>      exp_stat
%type   <compound_stat> compound_stat
%type   <stat_list>     stat_list
%type   <selection_stat> selection_stat
%type   <iteration_stat> iteration_stat
%type   <jump_stat>     jump_stat
%type   <exp>           exp
%type   <assignment_exp> assignment_exp
%type   <conditional_exp> conditional_exp
%type   <const_exp>     const_exp
%type   <logical_or_exp> logical_or_exp
%type   <logical_and_exp> logical_and_exp
%type   <inclusive_or_exp> inclusive_or_exp
%type   <exclusive_or_exp> exclusive_or_exp
%type   <and_exp>       and_exp
%type   <equality_exp>  equality_exp
%type   <relational_exp> relational_exp
%type   <shift_expression> shift_expression
%type   <additive_exp>  additive_exp
%type   <mult_exp>      mult_exp
%type   <cast_exp>      cast_exp
%type   <unary_exp>     unary_exp
%type   <unary_operator> unary_operator
%type   <postfix_exp>   postfix_exp
%type   <primary_exp>   primary_exp
%type   <literal>       literal
                        
%%

translation_unit	: external_decl
			| translation_unit external_decl
			;
external_decl		: function_definition
			| decl
			;
function_definition	: decl_specs declarator decl_list compound_stat
			|		declarator decl_list compound_stat
			| decl_specs declarator		compound_stat
			|		declarator 	compound_stat
			;
decl			: decl_specs init_declarator_list ';'
			| decl_specs			';'
			;
decl_list		: decl
			| decl_list decl
			;
decl_specs	: type_spec decl_specs
			| type_spec
			;
type_spec   : VOID | CHAR | SHORT | INT | LONG
			| SIGNED | UNSIGNED
			| struct_or_union_spec
			;
struct_or_union_spec	: struct_or_union ID '{' struct_decl_list '}'
			| struct_or_union	'{' struct_decl_list '}'
			| struct_or_union ID
			;
struct_or_union		: STRUCT | UNION
			;
struct_decl_list	: struct_decl
			| struct_decl_list struct_decl
			;
init_declarator_list	: init_declarator
			| init_declarator_list ',' init_declarator
			;
init_declarator		: declarator
			| declarator '=' initializer
			;
struct_decl		: spec_qualifier_list struct_declarator_list ';'
			;
spec_qualifier_list	: type_spec spec_qualifier_list
			| type_spec
			;
struct_declarator_list	: struct_declarator
			| struct_declarator_list ',' struct_declarator
			;
struct_declarator	: declarator
			| declarator ':' const_exp
			|		':' const_exp
			;
declarator		: pointer direct_declarator
			|	direct_declarator
			;
direct_declarator	: ID
			| '(' declarator ')'
			| direct_declarator '[' const_exp ']'
			| direct_declarator '['		']'
			| direct_declarator '(' param_type_list ')'
			| direct_declarator '(' id_list ')'
			| direct_declarator '('		')'
			;
pointer     : '*' 
			| '*'			pointer
			;
param_type_list		: param_list
			| param_list ',' LDOTS
			;
param_list		: param_decl
			| param_list ',' param_decl
			;
param_decl		: decl_specs declarator
			| decl_specs abstract_declarator
			| decl_specs
			;
id_list			: ID
			| id_list ',' ID
			;
initializer		: assignment_exp
			| '{' initializer_list '}'
			| '{' initializer_list ',' '}'
			;
initializer_list	: initializer
			| initializer_list ',' initializer
			;
type_name		: spec_qualifier_list abstract_declarator
			| spec_qualifier_list
			;
abstract_declarator	: pointer
			| pointer direct_abstract_declarator
			|	direct_abstract_declarator
			;
direct_abstract_declarator: '(' abstract_declarator ')'
			| direct_abstract_declarator '[' const_exp ']'
			|				'[' const_exp ']'
			| direct_abstract_declarator '['	']'
			|				'['	']'
			| direct_abstract_declarator '(' param_type_list ')'
			|				'(' param_type_list ')'
			| direct_abstract_declarator '('		')'
			|				'('		')'
			;
stat		: exp_stat
			| compound_stat
			| selection_stat
			| iteration_stat
			| jump_stat
			;
exp_stat		: exp ';'
			|	';'
			;
compound_stat		: '{' decl_list stat_list '}'
			| '{'		stat_list '}'
			| '{' decl_list		'}'
			| '{'			'}'
			;
stat_list		: stat
			| stat_list stat
			;
selection_stat	: IF '(' exp ')' stat
			| IF '(' exp ')' stat ELSE stat
			;
iteration_stat	: WHILE '(' exp ')' stat
			;
jump_stat	: GOTO ID ';'
			| CONTINUE ';'
			| BREAK ';'
			| RETURN exp ';'
			| RETURN ';'
			;

exp: assignment_exp
                ;

assignment_exp : unary_exp '=' assignment_exp
			;
conditional_exp:
                logical_or_exp
        |       logical_or_exp '?' exp ':' conditional_exp
        ;
const_exp		: conditional_exp
			;
logical_or_exp		: logical_and_exp
			| logical_or_exp OR logical_and_exp
			;
logical_and_exp		: inclusive_or_exp
			| logical_and_exp AND inclusive_or_exp
			;
inclusive_or_exp	: exclusive_or_exp
			| inclusive_or_exp '|' exclusive_or_exp
			;
exclusive_or_exp	: and_exp
			| exclusive_or_exp '^' and_exp
			;
and_exp			: equality_exp
			| and_exp '&' equality_exp
			;
equality_exp		: relational_exp
			| equalityexp EQ relational_exp
			| equality_exp NEQ relational_exp
			;
relational_exp		: shift_expression
			| relational_exp '<' shift_expression
			| relational_exp '>' shift_expression
			| relational_exp LEQ shift_expression
			| relational_exp GEQ shift_expression
			;
shift_expression	: additive_exp
			| shift_expression LSHIFT additive_exp
			| shift_expression RSHIFT additive_exp
			;
additive_exp		: mult_exp
			| additive_exp '+' mult_exp
			| additive_exp '-' mult_exp
			;
mult_exp		: cast_exp
			| mult_exp '*' cast_exp
			| mult_exp '/' cast_exp
			| mult_exp '%' cast_exp
			;
cast_exp		: unary_exp
			| '(' type_name ')' cast_exp
			;
unary_exp	: postfix_exp
			| INC unary_exp
			| DEC unary_exp
			| unary_operator cast_exp
			| SIZEOF unary_exp
			| SIZEOF '(' type_name ')'
			;
unary_operator		: '&' | '*' | '+' | '-' | '~' | '!'
			;
postfix_exp		: primary_exp
			| postfix_exp '[' exp ']'
			| postfix_exp '('			')'
			| postfix_exp '.' ID
			| postfix_exp ARROW ID
			| postfix_exp INC
			| postfix_exp DEC
			;
primary_exp		: ID
			| literal
			| STRING
			| '(' exp ')'
			;
literal     : INT_CONST
			| CHAR_CONST
			;
