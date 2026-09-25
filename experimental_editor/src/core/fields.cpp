#include "editor/core/fields.hpp"

#include <algorithm>
#include <utility>
#include <variant>

#include "compiler/models/operator.hpp"
#include "editor/core/identifier.hpp"
#include "editor/core/literal_text.hpp"
#include "editor/core/node_access.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/edit_call_argument.hpp"
#include "editor/transaction/edit_call_node.hpp"
#include "editor/transaction/edit_comment.hpp"
#include "editor/transaction/edit_operator.hpp"
#include "editor/transaction/rename.hpp"
#include "editor/transaction/set_constant_value.hpp"
#include "editor/transaction/update_func_param.hpp"

// A new text field adds a case to `read` and to `write`.

namespace fluir::editor::fields {
  namespace {

    template <typename T>
    const T* nodeOf(const et::ParseTree& tree, const FullID& path) {
      const et::Node* node = nodeAt(tree, path);
      return !node ? nullptr : std::get_if<T>(node);
    }

    const et::FunctionDecl::Parameter* paramAt(const et::FunctionDecl& fn, int index) {
      if (!fn.input) {
        return nullptr;
      }
      const auto it = std::ranges::find(fn.input->parameters, index, &et::FunctionDecl::Parameter::index);
      return it == fn.input->parameters.end() ? nullptr : &*it;
    }

    const et::FunctionDecl::Return* returnOf(const et::FunctionDecl* fn) {
      return fn && fn->output && fn->output->ret ? &*fn->output->ret : nullptr;
    }

    std::optional<fluir::Operator> operatorOf(const et::ParseTree& tree, const FullID& path) {
      if (const auto* binary = nodeOf<et::Binary>(tree, path)) {
        return binary->op;
      }
      const auto* unary = nodeOf<et::Unary>(tree, path);
      return !unary ? std::nullopt : std::optional{unary->op};
    }

    // The operator spelled `text`; BAR_BAR is the enum's last operator.
    std::optional<fluir::Operator> operatorNamed(const std::string& text) {
      for (int i = static_cast<int>(fluir::Operator::PLUS); i <= static_cast<int>(fluir::Operator::BAR_BAR); ++i) {
        if (stringify(static_cast<fluir::Operator>(i)) == text) {
          return static_cast<fluir::Operator>(i);
        }
      }
      return std::nullopt;
    }

    const et::Call::Argument* argumentAt(const et::Call& call, int index) {
      const auto it = std::ranges::find(call.arguments, index, &et::Call::Argument::index);
      return it == call.arguments.end() ? nullptr : &*it;
    }

  }  // namespace

  std::optional<std::string> read(const et::ParseTree& tree, const FullID& path, Field field) {
    switch (field.kind) {
      case Field::Kind::Name:
        {
          const et::FunctionDecl* fn = functionAt(tree, path);
          return !fn ? std::nullopt : std::optional{fn->name};
        }
      case Field::Kind::ParamName:
        {
          const et::FunctionDecl* fn = functionAt(tree, path);
          const et::FunctionDecl::Parameter* param = !fn ? nullptr : paramAt(*fn, field.index);
          return !param ? std::nullopt : std::optional{param->name};
        }
      case Field::Kind::ParamType:
        {
          const et::FunctionDecl* fn = functionAt(tree, path);
          const et::FunctionDecl::Parameter* param = !fn ? nullptr : paramAt(*fn, field.index);
          return !param ? std::nullopt : std::optional{param->typeName};
        }
      case Field::Kind::ReturnType:
        {
          const et::FunctionDecl::Return* ret = returnOf(functionAt(tree, path));
          return !ret ? std::nullopt : std::optional{ret->typeName};
        }
      case Field::Kind::Target:
        {
          const auto* call = nodeOf<et::Call>(tree, path);
          return !call ? std::nullopt : std::optional{call->target};
        }
      case Field::Kind::Arg:
        {
          const auto* call = nodeOf<et::Call>(tree, path);
          const et::Call::Argument* arg = !call ? nullptr : argumentAt(*call, field.index);
          return !arg ? std::nullopt : std::optional{arg->name};
        }
      case Field::Kind::Literal:
        {
          const auto* constant = nodeOf<et::Constant>(tree, path);
          return !constant || !isEditableLiteral(constant->value) ? std::nullopt :
                                                                    std::optional{renderLiteral(constant->value)};
        }
      case Field::Kind::Text:
        {
          const et::Comment* comment = commentAt(tree, path);
          return !comment ? std::nullopt : std::optional{comment->text};
        }
      case Field::Kind::Operator:
        {
          const std::optional<fluir::Operator> op = operatorOf(tree, path);
          return !op ? std::nullopt : std::optional{std::string{stringify(*op)}};
        }
      case Field::Kind::Bool:
        return std::nullopt;
    }
    return std::nullopt;
  }

  std::optional<std::unique_ptr<Transaction>> write(const et::ParseTree& tree,
                                                    const FullID& path,
                                                    Field field,
                                                    const std::string& text) {
    const std::optional<std::string> current = read(tree, path, field);
    if (!current) {
      return std::nullopt;
    }
    if (field.kind == Field::Kind::Literal) {
      const et::Literal& value = nodeOf<et::Constant>(tree, path)->value;
      const std::optional<et::Literal> parsed = tryParseLiteral(value, text);
      if (!parsed) {
        return std::nullopt;
      }
      return *parsed == value ? nullptr : setConstantValue(path, *parsed);
    }
    if (field.kind == Field::Kind::Operator) {
      const std::optional<fluir::Operator> op = operatorNamed(text);
      if (!op) {
        return std::nullopt;
      }
      return text == *current ? nullptr : setOperator(path, *op);
    }
    if (field.kind != Field::Kind::Text && !isValidIdentifier(text)) {
      return std::nullopt;
    }
    if (text == *current) {
      return nullptr;
    }
    switch (field.kind) {
      case Field::Kind::Name:
        return renameFunction(path, text);
      case Field::Kind::ParamName:
        return renameParameter(path, field.index, text);
      case Field::Kind::ParamType:
        return setRailType(path, paramAt(*functionAt(tree, path), field.index)->id, text);
      case Field::Kind::ReturnType:
        return setRailType(path, returnOf(functionAt(tree, path))->id, text);
      case Field::Kind::Target:
        return retargetCall(path, text);
      case Field::Kind::Arg:
        return renameCallArgument(path, field.index, text);
      case Field::Kind::Text:
        return editComment(path, text);
      default:
        return std::nullopt;
    }
  }

}  // namespace fluir::editor::fields
