#ifndef FLUIR_UTILITY_DIAGNOSTIC_CODE_HPP
#define FLUIR_UTILITY_DIAGNOSTIC_CODE_HPP

#include <cstdint>

namespace fluir {
  namespace diagnostic {
    enum class Code : std::uint16_t {
      GENERIC_NOTE = 0,
      GENERIC_WARNING = 0b1 << 14,
      // WARNINGS
      GENERIC_ERROR = 0b1 << 15,
      // ERRORS
    };
  }
}  // namespace fluir

#endif
