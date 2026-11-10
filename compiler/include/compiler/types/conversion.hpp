#ifndef FLUIR_COMPILER_TYPES_CONVERSION_HPP
#define FLUIR_COMPILER_TYPES_CONVERSION_HPP

#include <functional>

#include "compiler/types/type.hpp"

namespace fluir::types {
  struct Conversion {
    Type const* from;
    Type const* to;
    bool isImplicit{false};

    friend bool operator==(const Conversion& lhs, const Conversion& rhs) = default;
  };
}  // namespace fluir::types

namespace std {
  template <>
  struct hash<::fluir::types::Conversion> {
    size_t operator()(::fluir::types::Conversion const& conversion) const noexcept;
  };
}  // namespace std

#endif
