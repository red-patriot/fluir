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

  /** The description of a product type in the Fluir language */
  class Product : public Type {
   public:
    static Category classOf() { return Category::Product; }

    Product(std::string name, std::vector<TypeID> elements);

    [[nodiscard]] TypeID at(size_t index) const { return elements_.at(index); }

    friend bool operator==(const Product&, const Product&) = default;

   private:
    std::vector<TypeID> elements_;
  };
}  // namespace fluir::types

#endif
