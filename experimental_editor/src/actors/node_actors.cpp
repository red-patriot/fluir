#include "editor/actors/node_actors.hpp"

#include <cstdint>
#include <string>
#include <type_traits>
#include <variant>

#include <fmt/format.h>

#include "compiler/models/operator.hpp"

namespace fluir::editor {
  namespace {

    // Mirrors render_graph.cpp's private `renderLiteral`: I8 / U8 widen to int
    // so they print as numbers, BOOL prints true/false, and every other
    // arithmetic type goes straight through fmt.
    std::string renderLiteral(const pt::Literal& value) {
      return std::visit(
        [](auto v) -> std::string {
          using T = std::decay_t<decltype(v)>;
          if constexpr (std::is_same_v<T, bool>) {
            return v ? "true" : "false";
          } else if constexpr (std::is_same_v<T, std::int8_t> || std::is_same_v<T, std::uint8_t>) {
            return fmt::format("{}", static_cast<int>(v));
          } else {
            return fmt::format("{}", v);
          }
        },
        value);
    }

  }  // namespace

  void BinaryActor::onClick(Vec2) { lastClickSummary_ = fmt::format("binary {}", stringify(node_.op)); }

  void UnaryActor::onClick(Vec2) { lastClickSummary_ = fmt::format("unary {}", stringify(node_.op)); }

  void ConstantActor::onClick(Vec2) { lastClickSummary_ = fmt::format("constant {}", renderLiteral(node_.value)); }

  void CallActor::onClick(Vec2) { lastClickSummary_ = fmt::format("call {}", node_.target); }

}  // namespace fluir::editor
