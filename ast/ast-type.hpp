#ifndef __AST_HPP
#error "include \"ast.hpp\", not \"ast-type.hpp\" directly"
#endif

#ifndef __AST_TYPE_HPP
#define __AST_TYPE_HPP

#include "ast-size.hpp"

namespace zc {

   class FunctionType;
   class ASTType;

#if 0
   class BoundType: public ASTNode {
   public:
      Symbol *sym() const { return sym_; }
      ASTType *type() const { return type_; }

      virtual ASTType *
      
      template <typename... Args>
      static BoundType *Create(Args... args) { return new BoundType(args...); }
      
   protected:
      Symbol *sym_;
      ASTType *type_;

      template <typename... Args>
      BoundType(Symbol *sym, ASTType *type, Args... args):
         ASTNode(args...), sym_(sym), type_(type) {}
   }
#endif

   class ASTType: public ASTNode {
   public:
      enum class Kind {TYPE_BASIC,
                       TYPE_POINTER,
                       TYPE_FUNCTION,
                       TYPE_STRUCT};
      virtual Kind kind() const = 0;
      virtual const Decl *decl() const { return decl_; }
      Symbol *sym() const;
      virtual Size size() const = 0;

      virtual bool is_integral() const = 0;
      bool is_callable() const { return get_callable() != nullptr; }
      virtual const FunctionType *get_callable() const = 0;

      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {}

      void TypeCheck(SemantEnv& env) { TypeCheck(env, true); }
      virtual void TypeCheck(SemantEnv& env, bool allow_void) = 0;
      virtual bool TypeEq(const ASTType *other) const = 0;
      virtual bool TypeCoerce(const ASTType *from) const = 0;

      virtual ASTType *Address() = 0;
      virtual ASTType *Dereference(SemantEnv *env = nullptr) = 0;

      // void Enscope(SemantEnv& env) const;
      
   protected:
      /**
       * Associated declaration.
       */
      const Decl *decl_;

      ASTType(const SourceLoc& loc):
         ASTNode(loc), decl_(nullptr) {}
      ASTType(const Decl *decl, const SourceLoc& loc):
         ASTNode(loc), decl_(decl) {}
   };

   class BasicType: public ASTType {
   public:
      TypeSpec *type_spec() const { return type_spec_; }
      virtual Kind kind() const override { return Kind::TYPE_BASIC; }
      virtual bool is_integral() const override;
      virtual const FunctionType *get_callable() const override { return nullptr; }

      template <typename... Args>
      static BasicType *Create(Args... args) {
         return new BasicType(args...);
      }

      virtual void DumpNode(std::ostream& os) const override;

      virtual bool TypeEq(const ASTType *other) const override;
      virtual bool TypeCoerce(const ASTType *from) const override;
      virtual void TypeCheck(SemantEnv& env, bool allow_void) override;

      virtual ASTType *Address() override;
      virtual ASTType *Dereference(SemantEnv *env = nullptr) override;
      virtual Size size() const override;

      BasicType *Max(const BasicType *with) const;
       
   protected:
      TypeSpec *type_spec_;

      template <typename... Args>
      BasicType(TypeSpec *type_spec, Args... args):
         ASTType(args...), type_spec_(type_spec) {}
   };

   class Types: public ASTNodeVec<ASTType> {
   public:
      template <typename... Args>
      static Types *Create(Args... args) {
         return new Types(args...);
      }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {}
      
      bool TypeEq(const Types *others) const;
      void TypeCheck(SemantEnv& env);

   protected:
      Types(Vec vec, const SourceLoc& loc): ASTNodeVec<ASTType>(vec, loc) {}
      
   };

   class PointerType: public ASTType {
   public:
      virtual Kind kind() const override { return Kind::TYPE_POINTER; }
      virtual bool is_integral() const override { return false; }
      virtual const FunctionType *get_callable() const override;
      int depth() const { return depth_; }
      ASTType *pointee() const { return pointee_; }

      template <typename... Args>
      static PointerType *Create(Args... args) {
         return new PointerType(args...);
      }

      virtual void DumpNode(std::ostream& os) const override;

      virtual bool TypeEq(const ASTType *other) const override;
      virtual bool TypeCoerce(const ASTType *from) const override;
      virtual void TypeCheck(SemantEnv& env, bool allow_void) override;

      virtual ASTType *Address() override;
      virtual ASTType *Dereference(SemantEnv *env = nullptr) override;
      virtual Size size() const override { return Size::SZ_POINTER; }
      
   protected:
      int depth_;
      ASTType *pointee_;

      template <typename... Args>
      PointerType(int depth, ASTType *pointee, Args... args):
         ASTType(args...), depth_(depth), pointee_(pointee) {}
   };

   class FunctionType: public ASTType {
   public:
      virtual Kind kind() const override { return Kind::TYPE_FUNCTION; }
      virtual bool is_integral() const override { return false; }
      virtual const FunctionType *get_callable() const override { return this; }
      ASTType *return_type() const { return return_type_; }
      Types *params() const { return params_; }
      
      template <typename... Args>
      static FunctionType *Create(Args... args) {
         return new FunctionType(args...);
      }

      virtual void DumpNode(std::ostream& os) const override;

      virtual bool TypeEq(const ASTType *other) const override;
      virtual bool TypeCoerce(const ASTType *from) const override;
      virtual void TypeCheck(SemantEnv& env, bool allow_void) override;

      virtual ASTType *Address() override;
      virtual ASTType *Dereference(SemantEnv *env = nullptr) override;
      virtual Size size() const override {
         return Size::SZ_POINTER;
      }
      
   protected:
      ASTType *return_type_;
      Types *params_;

      template <typename... Args>
      FunctionType(ASTType *return_type, Types *params, Args... args):
         ASTType(args...), return_type_(return_type), params_(params) {}
   };

   class StructType: public ASTType {
   public:
      virtual Kind kind() const override { return Kind::TYPE_STRUCT; }
      virtual bool is_integral() const override { return false; }
      virtual const FunctionType *get_callable() const override { return nullptr; }
      Identifier *id() const { return id_; }
      Types *membs() const { return membs_; }

      template <typename... Args>
      static StructType *Create(Args... args) { return new StructType(args...); }

      virtual void DumpNode(std::ostream& os) const override;

      virtual bool TypeEq(const ASTType *other) const override;
      virtual bool TypeCoerce(const ASTType *from) const override;
      virtual void TypeCheck(SemantEnv& env, bool allow_void) override;

      virtual ASTType *Address() override;
      virtual ASTType *Dereference(SemantEnv *env = nullptr) override;
      virtual Size size() const override {
         throw std::logic_error("attempted to to get size enum of struct");
      }
      
   protected:
      Identifier *id_;
      Types *membs_;

      template <typename... Args>
      StructType(Identifier *id, Types *membs, Args... args):
         ASTType(args...), id_(id), membs_(membs) {}
   };

}
   
#endif
