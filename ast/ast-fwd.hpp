#ifndef __AST_HPP
#error "include ast.hpp, not ast/ast-fwd.h directly"
#endif

#ifndef __AST_FWD_HPP
#define __AST_FWD_HPP

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
   class ASTDeclarator;
   typedef ASTNodeVec<ASTDeclarator> ASTDeclarators;
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
