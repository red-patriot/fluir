#include "editor/core/intelligence.hpp"

#include <cstddef>
#include <format>
#include <ranges>
#include <string>
#include <variant>

#include "editor/core/fields.hpp"
#include "editor/core/tree_path.hpp"

namespace fluir::editor {
  namespace {
    // TODO: Dynamically populate these
    const std::vector<fluir::Operator> UNARY_OPERATORS{
      Operator::PLUS, Operator::MINUS, Operator::PLUS_PLUS, Operator::MINUS_MINUS, Operator::BANG};

    const std::vector<fluir::Operator> BINARY_OPERATORS{Operator::PLUS,
                                                        Operator::MINUS,
                                                        Operator::STAR,
                                                        Operator::SLASH,
                                                        Operator::EQUAL_EQUAL,
                                                        Operator::BANG_EQUAL,
                                                        Operator::GREATER_EQUAL,
                                                        Operator::LESS_EQUAL,
                                                        Operator::GREATER,
                                                        Operator::LESS,
                                                        Operator::AND_AND,
                                                        Operator::BAR_BAR};

    // TODO: Dynamically populate these
    const std::vector<std::string_view> BUILTIN_TYPES{
      "F64", "I8", "I16", "I32", "I64", "U8", "U16", "U32", "U64", "BOOL"};

    // TODO: Dynamically populate these
    const std::vector<Completion> TOP_LEVEL_COMPLETIONS{{"Function", FunctionDefOption{}},
                                                        {"Comment", CommentOption{}}};

    // One default-constructed (0 / false) value per Literal alternative, in order.
    template <typename... Ts>
    std::vector<literals_types::Literal> defaultsOf(const std::variant<Ts...>*) {
      return {literals_types::Literal{std::in_place_type<Ts>}...};
    }

  }  // namespace

  bool Intelligence::load(std::optional<std::filesystem::path>, const et::ParseTree& tree) {
    // TODO: Load other information about a module, for now just add parse its decls as available
    module_.functions.clear();
    for (const auto& [id, decl] : tree.declarations) {
      if (const auto* func = std::get_if<et::FunctionDecl>(&decl); func) {
        intelligence::FunctionDecl functionInfo;
        functionInfo.name = func->name;
        if (func->output && func->output->ret) {
          functionInfo.returnTypeName = func->output->ret->typeName;
        }
        if (func->input) {
          functionInfo.parameters.reserve(func->input->parameters.size());
          for (const auto& param : func->input->parameters) {
            functionInfo.parameters.push_back({param.name, param.typeName});
          }
        }
        module_.functions.emplace(id, std::move(functionInfo));
      }
    }

    return true;
  }

  bool Intelligence::unload(std::optional<std::filesystem::path> programPath) {
    // TODO: handle multiple modules at once
    module_.functions.clear();
    return true;
  }

  std::vector<std::string> Intelligence::choices(const et::ParseTree& tree, const FullID& path, Field field) const {
    if (!fields::read(tree, path, field)) {
      return {};
    }
    switch (field.kind) {
      case Field::Kind::Operator:
        {
          const bool unary = std::holds_alternative<et::Unary>(*nodeAt(tree, path));
          std::vector<std::string> out;
          for (const fluir::Operator op : unary ? UNARY_OPERATORS : BINARY_OPERATORS) {
            out.emplace_back(stringify(op));
          }
          return out;
        }
      case Field::Kind::ParamType:
      case Field::Kind::ReturnType:
        return {BUILTIN_TYPES.begin(), BUILTIN_TYPES.end()};
      default:
        return {};
    }
  }

  std::vector<Completion> Intelligence::completions(const et::ParseTree& tree, const FullID& body) const {
    if (body.empty()) {
      return TOP_LEVEL_COMPLETIONS;
    }
    const auto* block = blockOf(tree, body);
    return block ? completionsAt(*block) : std::vector<Completion>{};
  }

  std::vector<Completion> Intelligence::completionsAt(const et::Block&) const {
    auto options = bodyBuiltins();

    options.reserve(options.size() + module_.functions.size());
    for (const auto& func : module_.functions | std::ranges::views::values) {
      options.push_back({.label = std::format("{} (fn)", func.name),
                         .option = CallFunctionOption{
                           .target = func.name,
                           .parameters = func.parameters,
                           .returnType = func.returnTypeName,
                         }});
    }

    return options;
  }

  const std::vector<Completion>& Intelligence::bodyBuiltins() {
    // Completion labels are views, so the strings live here.
    static const std::vector<std::string> OPERATOR_LABELS = [] {
      std::vector<std::string> labels;
      for (Operator op : BINARY_OPERATORS) {
        labels.push_back(std::string{stringify(op)} + " (binary)");
      }
      for (Operator op : UNARY_OPERATORS) {
        labels.push_back(std::string{stringify(op)} + " (unary)");
      }
      return labels;
    }();
    static const std::vector<Completion> COMPLETIONS = [] {
      std::vector<Completion> out;
      std::size_t label = 0;
      for (Operator op : BINARY_OPERATORS) {
        out.push_back({OPERATOR_LABELS[label++], OperatorOption{op, OperatorOption::BINARY}});
      }
      for (Operator op : UNARY_OPERATORS) {
        out.push_back({OPERATOR_LABELS[label++], OperatorOption{op, OperatorOption::UNARY}});
      }
      // Builtin names follow Literal's alternative order.
      const auto defaults = defaultsOf(static_cast<const literals_types::Literal*>(nullptr));
      for (std::size_t i = 0; i < BUILTIN_TYPES.size(); ++i) {
        out.push_back({std::string{BUILTIN_TYPES[i]}, ConstantOption{defaults.at(i)}});
      }
      out.push_back({"Comment", CommentOption{}});
      out.push_back({"if-else", ConditionalOption{}});
      out.push_back({"print",
                     CallFunctionOption{.target = "print",
                                        .parameters = {intelligence::ParamInfo{.name = "object", .typeName = "ANY"}},
                                        .returnType = std::nullopt}});
      return out;
    }();
    return COMPLETIONS;
  }

}  // namespace fluir::editor
