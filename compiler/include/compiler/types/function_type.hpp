#ifndef FLUIR_COMPILER_TYPES_FUNCTION_TYPE_HPP
#define FLUIR_COMPILER_TYPES_FUNCTION_TYPE_HPP

#include <cstddef>
#include <functional>
#include <optional>
#include <vector>

#include "compiler/types/type.hpp"

namespace fluir::types {

  /** Represents the type signature of a function: its parameter types and optional return type */
  class FunctionType {
   public:
    FunctionType(std::vector<Type const*> parameters, Type const* returnType) :
      parameters_(std::move(parameters)), returnType_(returnType) { }

    [[nodiscard]] const std::vector<Type const*>& parameters() const { return parameters_; }
    [[nodiscard]] Type const* returnType() const { return returnType_; }

    friend bool operator==(const FunctionType&, const FunctionType&) = default;

   private:
    std::vector<Type const*> parameters_;
    Type const* returnType_;
  };

}  // namespace fluir::types

template <>
struct std::hash<fluir::types::FunctionType> {
  size_t operator()(const fluir::types::FunctionType& func) const noexcept;
};

#endif
