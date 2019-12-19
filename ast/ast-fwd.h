#ifndef __AST_H
#error "include ast.h, not ast/ast-fwd.h directly"
#endif

#ifndef __AST_FWD_H
#define __AST_FWD_H

namespace zc {

   /* typedefs */
   typedef std::size_t SourceLoc;
   
   /* forward declarations */
   template <class Node> class ASTNodeVec;
   class TranslationUnit;
   class Decl;
   typedef ASTNodeVec<Decl> Decls;
   class ExternalDecl;
   typedef ASTNodeVec<ExternalDecl> ExternalDecls;
   class DeclSpec;
   typedef ASTNodeVec<DeclSpec> DeclSpecs;
   class Declarator;
   typedef ASTNodeVec<Declarator> Declarators;
   class ParamDecl;
   typedef ASTNodeVec<ParamDecl> ParamDecls;
   class Stat;
   typedef ASTNodeVec<Stat> Stats;
   class FunctionDef;
   class CompoundStat;
   class TypeSpec;
   class Pointer;
   class DirectDeclarator;
   class ID;
   class ExprStat;
   class SelectionStat;
   class IterationStat;
   class JumpStat;
   class Expr;
   class AssignmentExpr;
   class UnaryExpr;
   class TypeName;
   class PostfixExpr;
   class UnaryCastExpr;
   class UnaryOperator;

   
   
}

#endif
