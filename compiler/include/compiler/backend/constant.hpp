#ifndef FLUIR_COMPILER_BACKEND_CONSTANT_HPP
#define FLUIR_COMPILER_BACKEND_CONSTANT_HPP

#include <cstdint>
#include <variant>
#include <vector>

namespace fluir::be {
  /** A value that can be written to the Constants section of bytecode */
  using Constant = std::variant<double,
                                std::int8_t,
                                std::int16_t,
                                std::int32_t,
                                std::int64_t,
                                std::uint8_t,
                                std::uint16_t,
                                std::uint32_t,
                                std::uint64_t>;

  /** An array of constants that forms a constants section */
  using ConstantsArray = std::vector<Constant>;
}  // namespace fluir::be

#endif
