#ifndef FLUIR_LSP_MODELS_LANGUAGE_HPP
#define FLUIR_LSP_MODELS_LANGUAGE_HPP

/** Public types for the lifecycle requests and responses of the LSP API */

#include <string>

namespace fluir::lsp::api {
    struct InitRequest {
    };

    struct InitResponse {
        std::string version;
    };

    struct ShutdownRequest {
    };

    struct ShutdownResponse {
    };
}

#endif
