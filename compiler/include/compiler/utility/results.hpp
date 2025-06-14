#ifndef FLUIR_COMPILER_UTILITY_RESULTS_HPP
#define FLUIR_COMPILER_UTILITY_RESULTS_HPP

#include <optional>

#include "diagnostics.hpp"

namespace fluir {
  template <typename T>
  using Results = std::optional<T>;

  constexpr auto NoResult = std::nullopt;
};  // namespace fluir

#endif
