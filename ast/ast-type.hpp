#ifndef __AST_HPP
#error "include \"ast.hpp\", not \"ast-type.hpp\" directly"
#endif

#ifndef __AST_TYPE_HPP
#define __AST_TYPE_HPP

#include <vector>

#include "ast-decl.hpp"

extern const char *g_filename;

namespace zc {


   class FunctionType;
   class ASTType;
   typedef std::vector<ASTType *> Types;

   bool TypeEq(const Types *lhs, const Types *rhs);   

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
      /**
       * The (not necessarily unique) tag name for this tagged type.
       */
      Symbol *tag() const { return tag_; }

      /**
       * The unique, scope-independent identifier for this tagged type.
       * All instances of this type shall have the same unique_id.
       */
      int unique_id() const { return unique_id_; }
      
      enum class TagKind {TAG_STRUCT, TAG_UNION, TAG_ENUM};
      virtual TagKind tag_kind() const = 0;
      virtual Kind kind() const override { return Kind::TYPE_TAGGED; }

      /**
       * Determine whether tagged type has been defined (not just declared).
       */
      virtual bool is_complete() const = 0;

      /**
       * Complete the definition of this tagged type instance given the definition of another
       * instance.
       * NOTE: Expects that @param other be of same derived type as this.
       */
      virtual void complete(const TaggedType *def) = 0;

      virtual void DumpNode(std::ostream& os) const override;
      
      virtual ASTType *Address() override;
      virtual ASTType *Dereference(SemantEnv *env) override;
      virtual void EnscopeTag(SemantEnv& env);      
      virtual void TypeCheck(SemantEnv& env, bool allow_void) override;
      virtual bool TypeEq(const ASTType *other) const override;      
      virtual void TypeCheckMembs(SemantEnv& env) = 0;
      
   protected:
      Symbol *tag_;
      int unique_id_;
      static int unique_id_counter_;

      virtual const char *name() const = 0;
      void AssignUniqueID(SemantEnv& env);

      template <typename... Args>
      TaggedType(Symbol *tag, Args... args): ASTType(args...), tag_(tag), unique_id_(-1) {}

      template <typename... Args>
      TaggedType(Symbol *tag, int unique_id, Args... args):
         ASTType(args...), tag_(tag), unique_id_(unique_id) {}
   };

   template <typename Memb>
   class TaggedType_aux: public TaggedType {
   public:
      typedef std::unordered_map<Symbol *, Memb *> Membs;
      Membs *membs() const { return membs_; }
      virtual bool is_complete() const override { return membs() != nullptr; }
      virtual void complete(const TaggedType *other) override {
         membs_ = dynamic_cast<const TaggedType_aux<Memb> *>(other)->membs();
      }

      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override {
         if (membs() != nullptr) {
            for (auto pair : *membs()) {
               pair.second->Dump(os, level, with_types);
            }
         }
      }
      
      virtual void TypeCheckMembs(SemantEnv& env) override {
         if (membs() != nullptr) {
            for (auto pair : *membs()) {
               pair.second->TypeCheck(env);
            }
         }
      }

   protected:
      Membs *membs_;

      template <typename InputIt, typename... Args>
      TaggedType_aux(SemantError& err, InputIt membs_begin, InputIt membs_end,
                     Args... args):
         TaggedType(args...), membs_(new Membs()) {
         for (; membs_begin != membs_end; ++membs_begin) {
            auto it = membs_->find((*membs_begin)->sym());
            if (it != membs_->end()) {
               err(g_filename, it->second) << "duplicate member '" << *it->second->sym() << "'"
                                           << std::endl;
            } else {
               (*membs_)[(*membs_begin)->sym()] = *membs_begin;
            }
         }
      }

      template <typename... Args>
      TaggedType_aux(Args... args): TaggedType(args...), membs_(nullptr) {}
   };

   /**
    * Abstract tagged type that has members (i.e. struct or union).
    */
   class CompoundType: public TaggedType_aux<ASTType> {
   public:
      virtual bool TypeCoerce(const ASTType *from) const override;

      virtual int bytes() const override = 0;
      virtual int offset(const Symbol *sym) const = 0;
      
   protected:
      template <typename... Args>
      CompoundType(Args... args): TaggedType_aux<ASTType>(args...) {}
   };
   
   class StructType: public CompoundType {
   public:
      virtual TagKind tag_kind() const override { return TagKind::TAG_STRUCT; }

      template <typename... Args>
      static StructType *Create(Args... args) { return new StructType(args...); }

      virtual int bytes() const override;
      virtual int offset(const Symbol *sym) const override;

   protected:
      virtual const char *name() const override { return "struct"; }

      template <typename... Args>
      StructType(Args... args): CompoundType(args...) {}
   };

   class UnionType: public CompoundType {
   public:
      virtual TagKind tag_kind() const override { return TagKind::TAG_UNION; }      

      template <typename... Args>
      static UnionType *Create(Args... args) { return new UnionType(args...); }

      virtual int bytes() const override;
      virtual int offset(const Symbol *sym) const override { return 0; }
      
   protected:
      virtual const char *name() const override { return "union"; }

      template <typename... Args>
      UnionType(Args... args): CompoundType(args...) {}
   };

   class Enumerator;
   class EnumType: public TaggedType_aux<Enumerator> {
   public:
      virtual TagKind tag_kind() const override {return TagKind::TAG_ENUM; }
      IntegralType *int_type() const { return int_type_; }

      virtual bool TypeCoerce(const ASTType *other) const override;

      virtual int bytes() const override { return int_type_->bytes(); }

      template <typename... Args>
      static EnumType *Create(Args... args) { return new EnumType(args...); }
      
   protected:
      IntegralType *int_type_;
      
      virtual const char *name() const override { return "enum"; }

      virtual void TypeCheckMembs(SemantEnv& env) override;
      
      template <typename... Args> EnumType(Args... args): TaggedType_aux<Enumerator>(args...) {
         int_type_ = IntegralType::Create(IntegralType::IntKind::SPEC_INT, 0);
      }
   };

   class Enumerator: public ASTNode {
   public:
      Identifier *id() const { return id_; }
      Symbol *sym() const { return id()->id(); }
      ASTExpr *val() const { return val_; }
      EnumType *enum_type() const { return enum_type_; }

      virtual void DumpNode(std::ostream& os) const override;
      virtual void DumpChildren(std::ostream& os, int level, bool with_types) const override;

      virtual void TypeCheck(SemantEnv& env) { TypeCheck(env, nullptr); }
      virtual void TypeCheck(SemantEnv& env, EnumType *enum_type);
      
      template <typename... Args>
      static Enumerator *Create(Args... args) { return new Enumerator(args...); }
      
   private:
      Identifier *id_;
      ASTExpr *val_;
      const Enumerator *prev_; /*!< Previous enumerator. Used for assigning values. */
      EnumType *enum_type_; /*!< Type, which shall be assigned during semantic analysis. */
      
      template <typename... Args>
      Enumerator(Identifier *id, ASTExpr *val, const Enumerator *prev, Args... args):
         ASTNode(args...), id_(id), val_(val), prev_(prev), enum_type_(nullptr) {}
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
   std::ostream& operator<<(std::ostream& os, CompoundType::TagKind kind);
   

}
   
#endif
