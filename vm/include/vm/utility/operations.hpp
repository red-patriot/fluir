#ifndef FLUIR_VM_UTILITY_OPERATIONS_HPP
#define FLUIR_VM_UTILITY_OPERATIONS_HPP

namespace fluir::utility {
  template <typename T>
  struct increment {
    constexpr T operator()(T& x) const { return ++x; }
  };

  template <typename T>
  struct decrement {
    constexpr T operator()(T& x) const { return --x; }
  };
}  // namespace fluir::utility

#endif
