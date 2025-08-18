#ifndef FLUIR_BYTECODE_VERSION_HPP
#define FLUIR_BYTECODE_VERSION_HPP

#include <cstdint>
#include <compare>

namespace fluir {
  static constexpr std::uint8_t VERSION_MAJOR = 0;
  static constexpr std::uint8_t VERSION_MINOR = 2;
  static constexpr std::uint8_t VERSION_PATCH = 0;

  struct Version {
    std::uint8_t major{VERSION_MAJOR};
    std::uint8_t minor{VERSION_MINOR};
    std::uint8_t patch{VERSION_PATCH};

    std::strong_ordering operator<=>(const Version&) const = default;
  };
}


#endif //VERSION_HPP
