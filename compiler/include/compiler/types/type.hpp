#ifndef FLUIR_COMPILER_TYPES_TYPE_HPP
#define FLUIR_COMPILER_TYPES_TYPE_HPP

#include <string>
#include <vector>

#include "compiler/types/typeid.hpp"

namespace fluir::types {
  enum class Category {
    Scalar,
    Product,
  };

  /** The description of a type in the Fluir language */
  class Type {
    // TODO: Create a Scalar type that is different from just a raw Type
   public:
    explicit Type(std::string name);
    virtual ~Type() = default;

    /** Access the unqualified name of this type */
    const std::string& name() const { return name_; }

    /** Returns true iff this type is of the indicated primary category */
    template <typename Concrete>
    [[nodiscard]] bool is() const {
      return Concrete::classOf() == category_;
    }
    template <typename Child>
      requires std::derived_from<Child, Type>
    Child* as() {
      return this->is<Child>() ? dynamic_cast<Child*>(this) : nullptr;
    }
    template <typename Child>
      requires std::derived_from<Child, Type>
    Child const* as() const {
      return this->is<Child>() ? dynamic_cast<Child const*>(this) : nullptr;
    }

    friend bool operator==(const Type&, const Type&) = default;

   protected:
    Type(std::string name, Category category);

   private:
    std::string name_;                    /**< The unqualified name of the type */
    Category category_{Category::Scalar}; /**< The primary category of this type */
  };

  class Scalar : public Type {
   public:
    friend const Scalar& F64();
    friend const Scalar& I8();
    friend const Scalar& I16();
    friend const Scalar& I32();
    friend const Scalar& I64();
    friend const Scalar& U8();
    friend const Scalar& U16();
    friend const Scalar& U32();
    friend const Scalar& U64();
    friend const Scalar& BOOL();
    // HACK: this is only used as an input type for builtins that handle typing "magically"
    // It represents a data sink and can accept any type
    // IT IS IMPERATIVE THIS NEVER BE ALLOWED TO LEAK INTO USER CODE
    // TODO: remove this once things are updated to support generics
    friend const Scalar& MAGIC_ANY_TYPE();

   private:
    Scalar(std::string name) : Type(std::move(name), Category::Scalar) { }
  };

  /** The description of a product type in the Fluir language */
  class Product : public Type {
   public:
    static Category classOf() { return Category::Product; }

    Product(std::string name, std::vector<TypeID> elements);

    [[nodiscard]] TypeID at(size_t index) const { return elements_.at(index); }
    /** Returns the number of elements in this product. */
    [[nodiscard]] size_t count() const { return elements_.size(); }
    /** Returns the number of elements in the product type*/
    [[nodiscard]] size_t elements() const { return elements_.size(); }

    /** Create a product type with the given elements. */
    static Product createFromTypes(std::vector<TypeID> elements);
    /** Create the name of a product type with the given elements. */
    static std::string nameFromTypes(const std::vector<TypeID>& elements);

    friend bool operator==(const Product&, const Product&) = default;

   private:
    std::vector<TypeID> elements_;
  };

  const Scalar& F64();
  const Scalar& I8();
  const Scalar& I16();
  const Scalar& I32();
  const Scalar& I64();
  const Scalar& U8();
  const Scalar& U16();
  const Scalar& U32();
  const Scalar& U64();
  const Scalar& BOOL();
  const Scalar& MAGIC_ANY_TYPE();
  const Product& EMPTY();
}  // namespace fluir::types

#endif
