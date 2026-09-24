#ifndef FLUIR_UTILITY_OVERLOADED_HPP
#define FLUIR_UTILITY_OVERLOADED_HPP

namespace fluir::util {
  /** One visitor from several lambdas, for `std::visit`. */
  template <typename... Fs>
  struct Overloaded : Fs... {
    using Fs::operator()...;
  };
}  // namespace fluir::util

#endif
