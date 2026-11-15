#include "compiler/types/conversion.hpp"

#include <cstdint>

namespace std {
  size_t hash<fluir::types::Conversion>::operator()(fluir::types::Conversion const& conversion) const noexcept {
    size_t seed = 0;
    constexpr hash<::fluir::types::TypePtr> h{};
    seed ^= h(conversion.from);
    seed ^= h(conversion.to);
    seed ^= std::hash<bool>{}(conversion.isImplicit);

    return seed;
  }

}  // namespace std
