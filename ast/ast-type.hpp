#ifndef __AST_HPP
#error "include \"ast.hpp\", not \"ast-type.hpp\" directly"
#endif

#ifndef __AST_TYPE_HPP
#define __AST_TYPE_HPP

namespace zc {

   class ASTType: public ASTNode {
   public:
      
   protected:
      ASTType(const SourceLoc& loc): ASTNode(loc) {}
   };

   class BasicType: public ASTType {
   public:
      static BasicType *Create(const SourceLoc& loc) {
         return new BasicType(loc);
      }
   protected:
      BasicType(const SourceLoc& loc): ASTType(loc) {}
   };

   const char ASTTypes_s[] = "ASTTypes";
   typedef ASTNodeVec<ASTType, ASTTypes_s> ASTTypes;

   class PointerType: public ASTType {
   public:
      int depth() const { return depth_; }
      ASTType *pointee() const { return pointee_; }

      static PointerType *Create(int depth, ASTType *subtype, const SourceLoc& loc) {
         return new PointerType(depth, subtype, loc);
      }
      
   protected:
      int depth_;
      ASTType *pointee_;
      
      PointerType(int depth, ASTType *pointee, const SourceLoc& loc):
         ASTType(loc), depth_(depth), pointee_(pointee) {}
   };

   class FunctionType: public ASTType {
   public:
      ASTType *return_type() const { return return_type_; }
      ASTTypes *params() const { return params_; }
      
      static FunctionType *Create(ASTType *return_type, ASTTypes *params, const SourceLoc& loc) {
         return new FunctionType(return_type, params, loc);
      }
      
   protected:
      ASTType *return_type_;
      ASTTypes *params_;

      FunctionType(ASTType *return_type, ASTTypes *params, const SourceLoc& loc):
         ASTType(loc), return_type_(return_type), params_(params) {}
   };

}
   
#endif
