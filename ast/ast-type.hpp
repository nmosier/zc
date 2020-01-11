#ifndef __AST_HPP
#error "include \"ast.hpp\", not \"ast-type.hpp\" directly"
#endif

#ifndef __AST_TYPE_HPP
#define __AST_TYPE_HPP

#include "ast-decl.hpp"

namespace zc {

   class FunctionType;
   class ASTType;

   class ASTType: public ASTNode {
   public:
      enum class Kind {TYPE_VOID,
                       TYPE_INTEGRAL,
                       TYPE_POINTER,
                       TYPE_FUNCTION,
                       TYPE_TAGGED,
                       TYPE_ARRAY};
      virtual Kind kind() const = 0;
      Symbol *sym() const { return sym_; }
      void set_sym(Symbol *sym) { sym_ = sym; }

      bool is_callable() const { return get_callable() != nullptr; }
      virtual const FunctionType *get_callable() const { return nullptr; }

      /**
       * Whether type contains another type (i.e. is a pointer or array).
       */
      bool is_container() const { return get_containee() != nullptr; }
      virtual ASTType *get_containee() const { return nullptr; }

      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {}

      void TypeCheck(SemantEnv& env) { TypeCheck(env, true); }
      virtual void TypeCheck(SemantEnv& env, bool allow_void) = 0;
      virtual bool TypeEq(const ASTType *other) const = 0;
      virtual bool TypeCoerce(const ASTType *from) const = 0;
      void Enscope(SemantEnv& env);

      virtual ASTType *Address() = 0;
      virtual ASTType *Dereference(SemantEnv *env = nullptr) = 0;

      void CodeGen(CgenEnv& env);
      virtual int bytes() const = 0;
      void FrameGen(StackFrame& frame) const;

      template <typename... Args>
      static ASTType *Create(Args... args) {
         return Decl::Create(args...)->Type();
      }
      
   protected:
      Symbol *sym_;

      template <typename... Args>
      ASTType(Args... args): ASTNode(args...), sym_(nullptr) {}
      template <typename... Args>
      ASTType(const Decl *decl, Args... args): ASTNode(args...), sym_(decl->id()->id()) {}
      template <typename... Args>
      ASTType(Symbol *sym, Args... args): ASTNode(args...), sym_(sym) {}

   };

   std::ostream& operator<<(std::ostream& os, ASTType::Kind kind);


   class Types: public ASTNodeVec<ASTType> {
   public:
      template <typename... Args>
      static Types *Create(Args... args) {
         return new Types(args...);
      }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {}

      ASTType *Lookup(const Symbol *sym) const;
      
      bool TypeEq(const Types *others) const;
      void TypeCheck(SemantEnv& env);
      void Enscope(SemantEnv& env);

      int bytes() const;

   protected:
      template <typename... Args>
      Types(Args... args): ASTNodeVec<ASTType>(args...) {}
      Types(Vec vec, const SourceLoc& loc): ASTNodeVec<ASTType>(vec, loc) {}
      
   };

   class PointerType: public ASTType {
   public:
      virtual Kind kind() const override { return Kind::TYPE_POINTER; }
      virtual const FunctionType *get_callable() const override;
      virtual ASTType *get_containee() const override { return pointee(); }
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

      virtual int bytes() const override;
      
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

      virtual int bytes() const override;
      
   protected:
      ASTType *return_type_;
      Types *params_;

      template <typename... Args>
      FunctionType(ASTType *return_type, Types *params, Args... args):
         ASTType(args...), return_type_(return_type), params_(params) {}
   };

   class VoidType: public ASTType {
   public:
      virtual Kind kind() const override { return Kind::TYPE_VOID; }
      
      virtual void DumpNode(std::ostream& os) const override { os << "VoidType VOID"; }
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {}

      virtual void TypeCheck(SemantEnv& env, bool allow_void) override;
      virtual bool TypeEq(const ASTType *other) const override;
      virtual bool TypeCoerce(const ASTType *from) const override { return TypeEq(from); }

      virtual ASTType *Address() override {
         throw std::logic_error("attempted to take address of 'void' type");
      }

      virtual ASTType *Dereference(SemantEnv *env) override {
         throw std::logic_error("attempted to dereference 'void' type");
      }

      virtual int bytes() const override;      
      
      template <typename... Args>
      static VoidType *Create(Args... args) { return new VoidType(args...); }
      
   protected:
      template <typename... Args>
      VoidType(Args... args): ASTType(args...) {}
   };

   class IntegralType: public ASTType {
   public:
      enum class IntKind {SPEC_CHAR, SPEC_SHORT, SPEC_INT, SPEC_LONG, SPEC_LONG_LONG};
      virtual Kind kind() const override { return Kind::TYPE_INTEGRAL; }
      IntKind int_kind() const { return int_kind_; }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {}

      virtual void TypeCheck(SemantEnv& env, bool allow_void) override {}
      virtual bool TypeEq(const ASTType *other)  const override;
      virtual bool TypeCoerce(const ASTType *from) const override { return true; }
      IntegralType *Max(const IntegralType *other) const;

      virtual ASTType *Address() override;
      virtual ASTType *Dereference(SemantEnv *env) override {
         throw std::logic_error("attempted to dereference integral type");
      }

      virtual int bytes() const override;      
      
      template <typename... Args>
      static IntegralType *Create(Args... args) { return new IntegralType(args...); }

   protected:
      IntKind int_kind_;

      template <typename... Args>
      IntegralType(IntKind int_kind, Args... args): ASTType(args...), int_kind_(int_kind) {}
   };

   class TaggedType: public ASTType {
   public:
      Symbol *tag() const { return tag_; }
      Types *membs() const { return membs_; }
      virtual Kind kind() const override { return Kind::TYPE_TAGGED; }
      enum class TagKind {TAG_STRUCT, TAG_UNION};
      virtual TagKind tag_kind() const = 0;

      virtual void DumpNode(std::ostream& os) const override;      
      virtual ASTType *Address() override;
      virtual ASTType *Dereference(SemantEnv *env) override;
      virtual bool TypeEq(const ASTType *other) const override;      
      virtual bool TypeCoerce(const ASTType *from) const override;
      virtual int bytes() const override = 0;
      virtual int offset(const Symbol *sym) const = 0;
      virtual void TypeCheck(SemantEnv& env, bool allow_void) override;
      void EnscopeTag(SemantEnv& env);      
      
   protected:
      Symbol *tag_;
      Types *membs_;

      virtual const char *name() const = 0;

      template <typename... Args>
      TaggedType(Symbol *tag, Types *membs, Args... args):
         ASTType(args...), tag_(tag), membs_(membs) {}
   };
   
   class StructType: public TaggedType {
   public:
      virtual TagKind tag_kind() const override { return TagKind::TAG_STRUCT; }

      template <typename... Args>
      static StructType *Create(Args... args) { return new StructType(args...); }

      virtual int bytes() const override;
      virtual int offset(const Symbol *sym) const override;

   protected:
      virtual const char *name() const override { return "struct"; }

      template <typename... Args>
      StructType(Args... args): TaggedType(args...) {}
   };

   class UnionType: public TaggedType {
   public:
      virtual TagKind tag_kind() const override { return TagKind::TAG_UNION; }      

      template <typename... Args>
      static UnionType *Create(Args... args) { return new UnionType(args...); }

      virtual int bytes() const override;
      virtual int offset(const Symbol *sym) const override { return 0; }
      
   protected:
      virtual const char *name() const override { return "union"; }

      template <typename... Args>
      UnionType(Args... args): TaggedType(args...) {}
   };

   class ArrayType: public ASTType {
   public:
      virtual Kind kind() const override { return Kind::TYPE_ARRAY; }
      ASTType *elem() const { return elem_; }
      ASTExpr *count() const { return count_; }
      intmax_t int_count() const { return int_count_; }
      virtual ASTType *get_containee() const override { return elem(); }            

      virtual void DumpNode(std::ostream& os) const override;

      virtual bool TypeEq(const ASTType *other) const override;
      virtual bool TypeCoerce(const ASTType *from) const override;
      virtual void TypeCheck(SemantEnv& env, bool allow_void) override;

      virtual ASTType *Address() override;
      virtual ASTType *Dereference(SemantEnv *env) override { return elem(); }

      virtual int bytes() const override;

      template <typename... Args>
      static ArrayType *Create(Args... args) { return new ArrayType(args...); }
      
   protected:
      ASTType *elem_;
      ASTExpr *count_; /*!< this expression must be constant */
      intmax_t int_count_; /*!< result after @see count_ is converted to 
                            * integer during semantic analysis */

      template <typename... Args>
      ArrayType(ASTType *elem, ASTExpr *count, Args... args):
         ASTType(args...), elem_(elem), count_(count) {}
   };

   std::ostream& operator<<(std::ostream& os, IntegralType::IntKind kind);
   std::ostream& operator<<(std::ostream& os, TaggedType::TagKind kind);

}
   
#endif
