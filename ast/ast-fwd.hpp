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
   template <class Spec, const char *name> class ASTSpecs;
   
   class TranslationUnit;
   class Decl;
   class Decls;
   class ExternalDecl;
   const char ExternalDecls_s[] = "ExternalDecls";
   typedef ASTNodeVec<ExternalDecl,ExternalDecls_s> ExternalDecls;
   class DeclSpecs;
   class ASTDeclarator;
   const char ASTDeclarators_s[] = "Declarators";
   typedef ASTNodeVec<ASTDeclarator,ASTDeclarators_s> ASTDeclarators;
   class FunctionDef;
   class CompoundStat;
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

   enum TypeSpec {TYPE_VOID, TYPE_CHAR, TYPE_SHORT, TYPE_INT, TYPE_LONG};
   const char constexpr TypeSpecs_s[] = "TypeSpecs";
   typedef ASTSpecs<TypeSpec,TypeSpecs_s> TypeSpecs;
   
}

#endif
