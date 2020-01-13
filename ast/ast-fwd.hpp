#ifndef __AST_HPP
#error "include ast.hpp, not ast/ast-fwd.h directly"
#endif

#ifndef __AST_FWD_HPP
#define __AST_FWD_HPP

#include <variant>
#include <vector>

namespace zc {

   /* typedefs */
   typedef std::size_t SourceLoc;
   
   /* forward declarations */
   class ASTNode;
   template <class Node> class ASTNodeVec;
   template <class Spec> class ASTSpecs;
   class ASTType;
   class DeclarableType;
   typedef std::vector<ASTType *> Types;

   class VarDeclaration;
   typedef std::vector<VarDeclaration *> VarDeclarations;
   
   
   class TranslationUnit;
   class Decl;
   class Decls;
   class ExternalDecl;
   typedef ASTNodeVec<ExternalDecl> ExternalDecls;

   class TypeDeclaration;
   typedef std::vector<TypeDeclaration *> TypeDeclarations;

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
   class TypeSpecs;
   class TypeSpec;
   class SemantEnv;
   class CgenEnv;
   class StackFrame;
   class FunctionImpl;
   class StructSpec;

   typedef std::variant<intmax_t, std::string> ConstVal;   

}

#endif
