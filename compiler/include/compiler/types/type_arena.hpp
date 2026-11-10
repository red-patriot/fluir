#ifndef FLUIR_COMPILER_TYPES_TYPE_ARENA_HPP
#define FLUIR_COMPILER_TYPES_TYPE_ARENA_HPP

#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "compiler/types/type.hpp"

namespace fluir::types {
  class TypeArena {
   public:
    TypeArena();

    /** Add or get a product type with the given elements.
     * If an identical product exists, gets the product instead.
     * \return The pointer to the product type.
     */
    Product const* addProduct(const std::vector<TypePtr>& elements);

    /** Searches types by name, */
    TypePtr findTypeNamed(std::string_view name) const;

   private:
    struct ElementwiseHash {
      using is_transparent = void;
      size_t operator()(std::span<TypePtr const> elements) const;
      size_t operator()(const std::unique_ptr<Product>& product) const { return operator()(product->elements()); }
    };
    struct ElementwiseEq {
      using is_transparent = void;
      bool operator()(std::span<TypePtr const> lhs, std::span<TypePtr const> rhs) const;
      bool operator()(std::span<TypePtr const> lhs, const std::unique_ptr<Product>& rhs) const {
        return operator()(lhs, rhs->elements());
      }
      bool operator()(const std::unique_ptr<Product>& lhs, std::span<TypePtr const> rhs) const {
        return operator()(lhs->elements(), rhs);
      }
      bool operator()(const std::unique_ptr<Product>& lhs, const std::unique_ptr<Product>& rhs) const {
        return operator()(lhs->elements(), rhs->elements());
      }
    };

    std::unordered_map<std::string_view, TypePtr> typeNames_{};
    std::unordered_set<std::unique_ptr<Product>, ElementwiseHash, ElementwiseEq> products_{};
  };
}  // namespace fluir::types

#endif
