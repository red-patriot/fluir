#ifndef FLUIR_COMPILER_TYPES_FUNCTION_TYPE_HPP
#define FLUIR_COMPILER_TYPES_FUNCTION_TYPE_HPP

#include <cstddef>
#include <functional>
#include <optional>
#include <vector>

#include "compiler/types/typeid.hpp"

namespace fluir::types {

  /** Represents the type signature of a function: its parameter types and optional return type */
  class FunctionType {
   public:
    FunctionType(std::vector<TypeID> parameters, std::optional<TypeID> returnType) :
      parameters_(std::move(parameters)), returnType_(returnType) { }

    [[nodiscard]] const std::vector<TypeID>& parameters() const { return parameters_; }
    [[nodiscard]] std::optional<TypeID> returnType() const { return returnType_; }

    friend bool operator==(const FunctionType&, const FunctionType&) = default;

   private:
    std::vector<TypeID> parameters_;
    std::optional<TypeID> returnType_;
  };

}  // namespace fluir::types

template <>
struct std::hash<fluir::types::FunctionType> {
  size_t operator()(const fluir::types::FunctionType& func) const noexcept;
};

#endif
