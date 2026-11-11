#include "editor/core/intelligence.hpp"

#include <cstddef>
#include <ranges>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "compiler/models/id.hpp"
#include "compiler/models/literal_types.hpp"
#include "editor/core/choices.hpp"
#include "editor/core/scope_path.hpp"

namespace fluir::editor {
  namespace {

    // A new conditional's condition port: both branches start empty, so inner id 1 is free; y clears the header band.
    constexpr pt::BlockPort CONDITION{.innerId = 1, .y = 10};

    // One default (0 / false) value per Literal alternative, in order.
    template <typename... Ts>
    std::vector<literals_types::Literal> defaultsOf(const std::variant<Ts...>*) {
      return {literals_types::Literal{std::in_place_type<Ts>}...};
    }

    pt::Comment comment() { return {.id = INVALID_ID, .location = {}, .text = ""}; }

    pt::Call callTo(std::string target, const std::vector<std::string>& parameters, bool returns) {
      pt::Call call{.id = INVALID_ID, .location = {}, .target = std::move(target), ._return = {}, .arguments = {}};
      for (std::size_t i = 0; i < parameters.size(); ++i) {
        call.arguments.push_back({.name = parameters[i], .index = static_cast<int>(i)});
      }
      if (returns) {
        call._return = pt::Call::Return{};
      }
      return call;
    }

  }  // namespace

  std::vector<Completion<pt::Declaration>> Intelligence::topLevelCompletions(const pt::ParseTree&) const {
    return {
      {"Function",
       pt::FunctionDecl{
         .id = INVALID_ID, .location = {}, .name = "new_function", .body = {}, .input = {}, .output = {}}},
      {"Comment", comment()},
    };
  }

  std::vector<Completion<pt::Node>> Intelligence::completionsAt(const pt::ParseTree& tree,
                                                                const fluir::FullID& scope) const {
    if (blockAt(tree, scope) == nullptr) {
      return {};
    }
    std::vector<Completion<pt::Node>> out;
    for (const std::string& op : binaryOperators()) {
      out.push_back({op + " (binary)", pt::Binary{.id = INVALID_ID, .location = {}, .op = *operatorNamed(op)}});
    }
    for (const std::string& op : unaryOperators()) {
      out.push_back({op + " (unary)", pt::Unary{.id = INVALID_ID, .location = {}, .op = *operatorNamed(op)}});
    }
    // Builtin type names follow Literal's alternative order.
    const auto defaults = defaultsOf(static_cast<const literals_types::Literal*>(nullptr));
    for (std::size_t i = 0; i < defaults.size(); ++i) {
      out.push_back({builtinTypes().at(i), pt::Constant{.id = INVALID_ID, .location = {}, .value = defaults[i]}});
    }
    out.push_back({"Comment", comment()});
    out.push_back({"if-else",
                   pt::Conditional{.id = INVALID_ID,
                                   .location = {},
                                   .condition = CONDITION,
                                   .inputs = {},
                                   .outputs = {},
                                   .thenScope = xyz::indirect<pt::Block>{},
                                   .elseScope = xyz::indirect<pt::Block>{}}});
    out.push_back({"print", callTo("print", {"object"}, false)});
    for (const pt::Declaration& decl : tree.declarations | std::views::values) {
      if (const auto* fn = std::get_if<pt::FunctionDecl>(&decl)) {
        std::vector<std::string> parameters;
        if (fn->input) {
          for (const pt::FunctionParameter& param : fn->input->parameters) {
            parameters.push_back(param.name);
          }
        }
        out.push_back({fn->name + " (fn)", callTo(fn->name, parameters, fn->output && fn->output->ret)});
      }
    }
    return out;
  }

}  // namespace fluir::editor
