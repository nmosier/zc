#ifndef __AST_HPP
#error "include \"ast.hpp\", not \"ast-type.hpp\" directly"
#endif

#ifndef __AST_TYPE_HPP
#define __AST_TYPE_HPP

namespace zc {

   class ASTType: public ASTNode {
   public:
      enum class Kind {TYPE_BASIC,
                       TYPE_POINTER,
                       TYPE_FUNCTION};
      virtual Kind kind() const = 0;

      virtual bool is_integral() const = 0;

      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {}

      virtual void TypeCheck(SemantEnv& env) = 0;
      virtual bool TypeEq(const ASTType *other) const = 0;
      virtual bool TypeCoerce(const ASTType *from) const = 0;

      virtual ASTType *Address() = 0;
      virtual ASTType *Dereference(SemantEnv *env = nullptr) = 0;
      
   protected:
      ASTType(const SourceLoc& loc): ASTNode(loc) {}
   };

   class BasicType: public ASTType {
   public:
      TypeSpec type_spec() const { return type_spec_; }
      virtual Kind kind() const override { return Kind::TYPE_BASIC; }
      virtual bool is_integral() const override { return IsIntegral(type_spec()); }
      
      static BasicType *Create(TypeSpec type_spec, const SourceLoc& loc) {
         return new BasicType(type_spec, loc);
      }

      virtual void DumpNode(std::ostream& os) const override;

      virtual bool TypeEq(const ASTType *other) const override;
      virtual bool TypeCoerce(const ASTType *from) const override;
      virtual void TypeCheck(SemantEnv& env) override;

      virtual ASTType *Address() override;
      virtual ASTType *Dereference(SemantEnv *env = nullptr) override;

      BasicType *Max(const BasicType *with) const;
       
   protected:
      TypeSpec type_spec_;
      
      BasicType(TypeSpec type_spec, const SourceLoc& loc): ASTType(loc), type_spec_(type_spec) {}
   };

   const char Types_s[] = "ASTTypes";
   class Types: public ASTNodeVec<ASTType, Types_s> {
   public:      
      static Types *Create(Vec vec, const SourceLoc& loc) { return new Types(vec, loc); }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {}
      
      bool TypeEq(const Types *others) const;
      void TypeCheck(SemantEnv& env);

   protected:
      Types(Vec vec, const SourceLoc& loc): ASTNode(loc), ASTNodeVec<ASTType, Types_s>(vec, loc) {}
      
   };

   class PointerType: public ASTType {
   public:
      virtual Kind kind() const override { return Kind::TYPE_POINTER; }
      virtual bool is_integral() const override { return false; }      
      int depth() const { return depth_; }
      ASTType *pointee() const { return pointee_; }

      static PointerType *Create(int depth, ASTType *pointee, const SourceLoc& loc) {
         return new PointerType(depth, pointee, loc);
      }

      virtual void DumpNode(std::ostream& os) const override;

      virtual bool TypeEq(const ASTType *other) const override;
      virtual bool TypeCoerce(const ASTType *from) const override;
      virtual void TypeCheck(SemantEnv& env) override;

      virtual ASTType *Address() override;
      virtual ASTType *Dereference(SemantEnv *env = nullptr) override;
      
   protected:
      int depth_;
      ASTType *pointee_;
      
      PointerType(int depth, ASTType *pointee, const SourceLoc& loc):
         ASTType(loc), depth_(depth), pointee_(pointee) {}
   };

   class FunctionType: public ASTType {
   public:
      virtual Kind kind() const override { return Kind::TYPE_FUNCTION; }
      virtual bool is_integral() const override { return false; }
      ASTType *return_type() const { return return_type_; }
      Types *params() const { return params_; }
      
      
      static FunctionType *Create(ASTType *return_type, Types *params, const SourceLoc& loc) {
         return new FunctionType(return_type, params, loc);
      }

      virtual void DumpNode(std::ostream& os) const override;

      virtual bool TypeEq(const ASTType *other) const override;
      virtual bool TypeCoerce(const ASTType *from) const override;
      virtual void TypeCheck(SemantEnv& env) override;      

      virtual ASTType *Address() override;
      virtual ASTType *Dereference(SemantEnv *env = nullptr) override;
      
   protected:
      ASTType *return_type_;
      Types *params_;



      FunctionType(ASTType *return_type, Types *params, const SourceLoc& loc):
         ASTType(loc), return_type_(return_type), params_(params) {}
   };

}
   
#endif
