#ifndef FLUIR_LSP_TEST_JSON_UTILS_HPP
#define FLUIR_LSP_TEST_JSON_UTILS_HPP

#include <ranges>

#include <nlohmann/json.hpp>

#include "lsp/json_serialize.hpp"

namespace fluir::lsp::test {

  template <std::ranges::range R>
  nlohmann::json toJsonArray(const R& range) {
    auto arr = nlohmann::json::array();
    for (const auto& elem : range) {
      arr.push_back(toJson(elem));
    }
    return arr;
  }

}  // namespace fluir::lsp::test

#endif
