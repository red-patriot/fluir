#include "compiler/types/function_type.hpp"

#include <cstdint>
#include <functional>

size_t std::hash<fluir::types::FunctionType>::operator()(const fluir::types::FunctionType& func) const noexcept {
  using fluir::types::TypeID;
  size_t seed = func.returnType() ? std::hash<std::uint64_t>{}(static_cast<std::uint64_t>(*func.returnType())) : 0;
  for (auto id : func.parameters()) {
    seed ^= std::hash<std::uint64_t>{}(static_cast<std::uint64_t>(id)) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
  }
  return seed;
}
