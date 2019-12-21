/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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
/* Line 1529 of yacc.c.  */
#line 114 "c.tab.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

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

extern YYLTYPE yylloc;
