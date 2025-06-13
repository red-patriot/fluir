module;

#include <concepts>
#include <functional>
#include <type_traits>

export module fluir.utility.pass;
import fluir.utility.context;

namespace fluir {
  export template <typename T, typename U>
  concept CompilerPass = requires(Context& ctx, U u) {
    { T::run(ctx, u) };
  };
}  // namespace fluir
