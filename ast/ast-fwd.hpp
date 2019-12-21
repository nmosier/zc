#ifndef __AST_HPP
#error "include ast.hpp, not ast/ast-fwd.h directly"
#endif

#ifndef __AST_FWD_HPP
#define __AST_FWD_HPP

namespace zc {

   /* typedefs */
   typedef std::size_t SourceLoc;
   
   /* forward declarations */
   class ASTNode;
   template <class Node, const char *name> class ASTNodeVec;
   class TranslationUnit;
   class Decl;
   class Decls;
   class ExternalDecl;
   const char ExternalDecls_s[] = "ExternalDecls";
   typedef ASTNodeVec<ExternalDecl,ExternalDecls_s> ExternalDecls;
   class DeclSpec;
   const char DeclSpecs_s[] = "DeclSpecs";
   typedef ASTNodeVec<DeclSpec,DeclSpecs_s> DeclSpecs;
   class ASTDeclarator;
   const char ASTDeclarators_s[] = "Declarators";
   typedef ASTNodeVec<ASTDeclarator,ASTDeclarators_s> ASTDeclarators;
   class FunctionDef;
   class CompoundStat;
   class TypeSpec;
   class Pointer;
   class Identifier;
   class ExprStat;
   class SelectionStat;
   class IterationStat;
   class JumpStat;
   class AssignmentExpr;
   class UnaryExpr;
   class TypeName;
   class PostfixExpr;
   class UnaryCastExpr;
   class UnaryOperator;
   class ConditionalExpr;
   class LogicalOrExpr;
   class LogicalAndExpr;
}

#endif
