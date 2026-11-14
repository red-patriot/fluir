#include "compiler/types/function_type.hpp"

#include <cstdint>
#include <functional>

size_t std::hash<fluir::types::FunctionType>::operator()(const fluir::types::FunctionType& func) const noexcept {
  size_t seed = std::hash<::fluir::types::TypePtr>{}(func.returnType());
  for (auto param : func.parameters()) {
    seed ^= std::hash<::fluir::types::TypePtr>{}(param) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
  }
  return seed;
}
