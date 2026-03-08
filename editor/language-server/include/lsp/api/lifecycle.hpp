#ifndef FLUIR_LSP_API_LIFECYCLE_HPP
#define FLUIR_LSP_API_LIFECYCLE_HPP

/** Public types for the lifecycle requests and responses of the LSP API */

#include <string>

namespace fluir::lsp::api {
  struct InitRequest { };

  struct InitResponse {
    std::string version;
  };

  struct ShutdownRequest { };

  struct ShutdownResponse { };
}  // namespace fluir::lsp::api

#endif
