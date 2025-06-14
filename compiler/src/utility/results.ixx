module;

#include <optional>

export module fluir.utility.results;

namespace fluir {
  export template <typename T>
  using Results = std::optional<T>;

  export constexpr auto NoResult = std::nullopt;
};  // namespace fluir
