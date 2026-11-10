#ifndef FLUIR_COMPILER_TYPES_FUNCTION_TYPE_HPP
#define FLUIR_COMPILER_TYPES_FUNCTION_TYPE_HPP

#include <cstddef>
#include <functional>
#include <optional>
#include <vector>

#include "compiler/types/type.hpp"

namespace fluir::types {

  /** Represents the type signature of a function */
  class FunctionType {
    // TODO: Fold this into the existing type system
   public:
    FunctionType(std::vector<TypePtr> parameters, TypePtr returnType) :
      parameters_(std::move(parameters)), returnType_(returnType) { }

    [[nodiscard]] const std::vector<TypePtr>& parameters() const { return parameters_; }
    [[nodiscard]] TypePtr returnType() const { return returnType_; }

    friend bool operator==(const FunctionType&, const FunctionType&) = default;

   private:
    std::vector<TypePtr> parameters_;
    TypePtr returnType_;
  };

  using FunctionTypePtr = FunctionType const*;

}  // namespace fluir::types

template <>
struct std::hash<fluir::types::FunctionType> {
  size_t operator()(const fluir::types::FunctionType& func) const noexcept;
};

#endif
