#include "compiler/debug/parse_tree_printer.hpp"

#include <algorithm>
#include <vector>
namespace fluir::debug {
  namespace {
    template <typename ValueT>
    std::vector<ID> keyOrder(const std::unordered_map<ID, ValueT>& data) {
      std::vector<ID> keys;
      keys.reserve(data.size());
      for (const auto& [key, _] : data) {
        keys.push_back(key);
      }
      std::ranges::sort(keys);
      return keys;
    }

    std::string_view toString(pt::Constant::Type t) {
      switch (t) {
        case pt::Constant::Type::F64:
          return "F64";
        case pt::Constant::Type::I8:
          return "I8";
        case pt::Constant::Type::I16:
          return "I16";
        case pt::Constant::Type::I32:
          return "I32";
        case pt::Constant::Type::I64:
          return "I64";
        case pt::Constant::Type::U8:
          return "U8";
        case pt::Constant::Type::U16:
          return "U16";
        case pt::Constant::Type::U32:
          return "U32";
        case pt::Constant::Type::U64:
          return "U64";
        case pt::Constant::Type::BOOL:
          return "BOOL";
      }
      return "<UNKNOWN TYPE>";
    }
  }  // namespace

  ParseTreePrinter::ParseTreePrinter(std::ostream& out) : out_(out) { }

  void ParseTreePrinter::print(const pt::ParseTree& tree) {
    auto orderedKeys = keyOrder(tree.declarations);

    for (const auto& key : orderedKeys) {
      std::visit(*this, tree.declarations.at(key));
    }
  }

  void ParseTreePrinter::operator()(const pt::FunctionDecl& func) {
    out_ << formatIndented("{}:\n", func.id);
    FLUIR_SCOPED_INDENT;
    out_ << formatIndented("FunctionDecl({})\n", func.name) << doPrint(func.location);

    if (func.input) {
      (*this)(*func.input);
    }
    if (func.output) {
      (*this)(*func.output);
    }

    printBlock(func.body);
  }

  void ParseTreePrinter::operator()(const pt::FunctionDecl::Parameter& param) {
    out_ << formatIndented("{}:\n", param.id);
    FLUIR_SCOPED_INDENT;
    out_ << formatIndented("Param({})\n", param.name) << formatIndented("type {}\n", param.typeName);
  }
  void ParseTreePrinter::operator()(const pt::FunctionDecl::Return& ret) {
    out_ << formatIndented("{}:\n", ret.id);
    FLUIR_SCOPED_INDENT;
    out_ << formatIndented("Return\n") << formatIndented("type {}\n", ret.typeName);
  }

  void ParseTreePrinter::operator()(const pt::FunctionDecl::InputBlock& input) {
    out_ << formatIndented("input\n");
    FLUIR_SCOPED_INDENT;
    for (const auto& param : input.parameters) {
      (*this)(param);
    }
  }

  void ParseTreePrinter::operator()(const pt::FunctionDecl::OutputBlock& output) {
    out_ << formatIndented("output\n");
    FLUIR_SCOPED_INDENT;
    if (output.ret) {
      (*this)(*output.ret);
    }
  }

  void ParseTreePrinter::operator()(const pt::Binary& binary) {
    out_ << formatIndented("{}:\n", binary.id);
    FLUIR_SCOPED_INDENT;
    out_ << formatIndented("Binary\n") << doPrint(binary.location) << formatIndented("{}\n", stringify(binary.op))
         << formatIndented("lhs{}\n", binary.lhs) << formatIndented("rhs{}\n", binary.rhs);
  }
  void ParseTreePrinter::operator()(const pt::Unary& unary) {
    out_ << formatIndented("{}:\n", unary.id);
    FLUIR_SCOPED_INDENT;
    out_ << formatIndented("Unary\n") << doPrint(unary.location) << formatIndented("{}\n", stringify(unary.op))
         << formatIndented("lhs{}\n", unary.lhs);
  }
  void ParseTreePrinter::operator()(const pt::Constant& constant) {
    out_ << formatIndented("{}:\n", constant.id);
    FLUIR_SCOPED_INDENT;
    out_ << formatIndented("Constant\n") << doPrint(constant.location);
    out_ << formatIndented("{} {}\n", toString(constant.typeTag), constant.value);
  }

  void ParseTreePrinter::operator()(const pt::Call& call) {
    out_ << formatIndented("{}:\n", call.id);
    FLUIR_SCOPED_INDENT;
    out_ << formatIndented("Call({})\n", call.target) << doPrint(call.location);
    if (call._return) {
      out_ << formatIndented("out:\n");
      FLUIR_SCOPED_INDENT;
      out_ << formatIndented("0: return\n");
    }
    if (const auto& args = call.arguments; !args.empty()) {
      out_ << formatIndented("in:\n");
      FLUIR_SCOPED_INDENT;
      for (const auto& [name, index] : args) {
        out_ << formatIndented("{}: {}\n", index, name);
      }
    }
  }

  void ParseTreePrinter::operator()(const pt::Conditional& conditional) {
    out_ << formatIndented("{}:\n", conditional.id);
    FLUIR_SCOPED_INDENT;
    out_ << formatIndented("Conditional\n") << doPrint(conditional.location);
    const auto& condition = conditional.condition;
    out_ << formatIndented("Condition {} at({})\n", condition.innerId, condition.y);
    {
      out_ << formatIndented("input\n");
      FLUIR_SCOPED_INDENT;
      for (const auto& port : conditional.inputs) {
        out_ << formatIndented("{} at({})\n", port.innerId, port.y);
      }
    }
    {
      out_ << formatIndented("output\n");
      FLUIR_SCOPED_INDENT;
      for (const auto& port : conditional.outputs) {
        out_ << formatIndented("{} at({})\n", port.innerId, port.y);
      }
    }
    const auto& then = conditional.thenScope;
    out_ << formatIndented("Then\n");
    printBlock(*then);

    const auto& else_ = conditional.elseScope;
    out_ << formatIndented("Else\n");
    printBlock(*else_);
  }

  void ParseTreePrinter::operator()(const pt::Conduit& conduit) {
    out_ << formatIndented("{}:\n", conduit.id);
    FLUIR_SCOPED_INDENT;
    out_ << formatIndented("Conduit\n") << formatIndented("in{}.{}\n", conduit.input, conduit.index)
         << formatIndented("children\n");
    {
      FLUIR_SCOPED_INDENT;
      for (const auto& child : conduit.children) {
        out_ << formatIndented("out{}.{}\n", child.target, child.index);
      }
    }
  }

  void ParseTreePrinter::operator()(const pt::Comment& comment) {
    out_ << formatIndented("{}:\n", comment.id);
    FLUIR_SCOPED_INDENT;
    out_ << formatIndented("Comment\n") << doPrint(comment.location);
    out_ << formatIndented("{{{{") << comment.text << "}}\n";
  }

  std::string ParseTreePrinter::doPrint(const FlowGraphLocation& loc) {
    return formatIndented("at(x{}, y{}, z{}, w{}, h{})\n", loc.x, loc.y, loc.z, loc.width, loc.height);
  }

  void ParseTreePrinter::printBlock(const pt::Block& block) {
    {
      out_ << formatIndented("body\n");
      auto orderedNodes = keyOrder(block.nodes);
      FLUIR_SCOPED_INDENT;
      for (const auto& node : orderedNodes) {
        std::visit(*this, block.nodes.at(node));
      }
    }

    {
      out_ << formatIndented("conduits\n");
      auto orderedConduits = keyOrder(block.conduits);
      FLUIR_SCOPED_INDENT;
      for (const auto& conduit : orderedConduits) {
        (*this)(block.conduits.at(conduit));
      }
    }
  }

}  // namespace fluir::debug
