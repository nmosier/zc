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
   template <class Node> class ASTNodeVec;
   template <class Spec> class ASTSpecs;
   class ASTType;
   
   class TranslationUnit;
   class Decl;
   class Decls;
   class ExternalDecl;
   typedef ASTNodeVec<ExternalDecl> ExternalDecls;

   class DeclSpecs;
   class ASTDeclarator;
   typedef ASTNodeVec<ASTDeclarator> ASTDeclarators;
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

   /** Collection of type specification categories */
   enum class TypeSpec {TYPE_VOID,
                        TYPE_CHAR,
                        TYPE_SHORT,
                        TYPE_INT,
                        TYPE_LONG,
                        TYPE_LONG_LONG,
   };
   bool IsIntegral(TypeSpec type);
   TypeSpec Max(TypeSpec lhs, TypeSpec rhs);
   
   class TypeSpecs;

   class SemantEnv;
   class CgenEnv;
   class StackFrame;
   class FunctionImpl;
}

#endif
