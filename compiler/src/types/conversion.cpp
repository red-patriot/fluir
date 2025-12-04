#include "compiler/types/conversion.hpp"

#include <cstdint>

namespace std {
  size_t hash<fluir::types::Conversion>::operator()(fluir::types::Conversion const& conversion) const noexcept {
    size_t seed = 0;
    constexpr hash<::fluir::types::TypeID> enumHash{};
    seed ^= enumHash(conversion.from);
    seed ^= enumHash(conversion.to);
    seed ^= std::hash<bool>{}(conversion.isImplicit);

    return seed;
  }

}  // namespace std
