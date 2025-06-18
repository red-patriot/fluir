export module fluir.utility.context;
export import fluir.utility.diagnostics;
export import fluir.utility.results;

namespace fluir {
  /** The context of compilation. */
  export struct Context {
    Diagnostics diagnostics{}; /**< The diagnostics produced by the compilation process */
  };
}  // namespace fluir
