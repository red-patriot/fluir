#ifndef FLUIR_LSP_API_DOCUMENT_HPP
#define FLUIR_LSP_API_DOCUMENT_HPP

/** Public types for the document requests and responses of the LSP API */
#include <string>

namespace fluir::lsp::api {
    struct OpenDocRequest {
        std::string path;
        std::string content;
    };

    struct OpenDocResponse {
    };

    struct CloseDocRequest {
        std::string path;
    };

    struct CloseDocResponse {
    };

    struct DocEdit {
        std::string contents;
    };

    struct DocEditResponse {
    };
}

#endif
