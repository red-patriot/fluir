#ifndef FLUIR_LSP_TEST_FILE_UTILITY_HPP
#define FLUIR_LSP_TEST_FILE_UTILITY_HPP

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

namespace fluir::lsp::test {
  inline nlohmann::json readTestJson(const std::filesystem::path& relative) {
    auto path = std::filesystem::path{ROOT_LSP_TEST_DIR} / "test_programs" / relative;
    std::ifstream fin(path);
    return nlohmann::json::parse(fin);
  }
}  // namespace fluir::lsp::test

#endif
