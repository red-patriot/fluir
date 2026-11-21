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
    const auto& defaults = defaultScalars();
    std::ranges::copy(defaults, std::back_inserter(out));
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

  const std::vector<Completion<pt::Node>>& Intelligence::defaultScalars() {
    using NodeComp = Completion<pt::Node>;
    static const std::vector defaults{
      NodeComp{
        .label = "F64",
        .prototype = pt::Constant{.id = INVALID_ID, .location = {}, .value = "0.0", .typeTag = pt::Constant::F64}},
      NodeComp{.label = "I8",
               .prototype = pt::Constant{.id = INVALID_ID, .location = {}, .value = "0", .typeTag = pt::Constant::I8}},
      NodeComp{.label = "I16",
               .prototype = pt::Constant{.id = INVALID_ID, .location = {}, .value = "0", .typeTag = pt::Constant::I16}},
      NodeComp{.label = "I32",
               .prototype = pt::Constant{.id = INVALID_ID, .location = {}, .value = "0", .typeTag = pt::Constant::I32}},
      NodeComp{.label = "I64",
               .prototype = pt::Constant{.id = INVALID_ID, .location = {}, .value = "0", .typeTag = pt::Constant::I64}},
      NodeComp{.label = "U8",
               .prototype = pt::Constant{.id = INVALID_ID, .location = {}, .value = "0", .typeTag = pt::Constant::U8}},
      NodeComp{.label = "U16",
               .prototype = pt::Constant{.id = INVALID_ID, .location = {}, .value = "0", .typeTag = pt::Constant::U16}},
      NodeComp{.label = "U32",
               .prototype = pt::Constant{.id = INVALID_ID, .location = {}, .value = "0", .typeTag = pt::Constant::U32}},
      NodeComp{.label = "U64",
               .prototype = pt::Constant{.id = INVALID_ID, .location = {}, .value = "0", .typeTag = pt::Constant::U64}},
      NodeComp{
        .label = "false",
        .prototype = pt::Constant{.id = INVALID_ID, .location = {}, .value = "false", .typeTag = pt::Constant::BOOL}},
      NodeComp{
        .label = "true",
        .prototype = pt::Constant{.id = INVALID_ID, .location = {}, .value = "true", .typeTag = pt::Constant::BOOL}}};

    return defaults;
  }

}  // namespace fluir::editor
