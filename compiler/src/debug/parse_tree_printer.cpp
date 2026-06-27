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

    {
      out_ << formatIndented("body\n");
      auto orderedNodes = keyOrder(func.body.nodes);
      FLUIR_SCOPED_INDENT;
      for (const auto& node : orderedNodes) {
        std::visit(*this, func.body.nodes.at(node));
      }
    }

    {
      out_ << formatIndented("conduits\n");
      auto orderedConduits = keyOrder(func.body.conduits);
      FLUIR_SCOPED_INDENT;
      for (const auto& conduit : orderedConduits) {
        (*this)(func.body.conduits.at(conduit));
      }
    }
  }

  void ParseTreePrinter::operator()(const pt::FunctionDecl::Parameter& param) {
    out_ << formatIndented("{}:\n", param.id);
    FLUIR_SCOPED_INDENT;
    out_ << formatIndented("Param({})\n", param.name) << formatIndented("index {}\n", param.index)
         << formatIndented("type {}\n", param.typeName);
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
    std::visit(*this, constant.value);
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

  void ParseTreePrinter::operator()(const pt::F64& f64) { out_ << formatIndented("F64 {}\n", f64); }
  void ParseTreePrinter::operator()(const pt::I8& i8) { out_ << formatIndented("I8 {}\n", i8); }
  void ParseTreePrinter::operator()(const pt::I16& i16) { out_ << formatIndented("I16 {}\n", i16); }
  void ParseTreePrinter::operator()(const pt::I32& i32) { out_ << formatIndented("I32 {}\n", i32); }
  void ParseTreePrinter::operator()(const pt::I64& i64) { out_ << formatIndented("I64 {}\n", i64); }
  void ParseTreePrinter::operator()(const pt::U8& u8) { out_ << formatIndented("U8 {}\n", u8); }
  void ParseTreePrinter::operator()(const pt::U16& u16) { out_ << formatIndented("U16 {}\n", u16); }
  void ParseTreePrinter::operator()(const pt::U32& u32) { out_ << formatIndented("U32 {}\n", u32); }
  void ParseTreePrinter::operator()(const pt::U64& u64) { out_ << formatIndented("U64 {}\n", u64); }
  void ParseTreePrinter::operator()(const pt::BOOL& b) { out_ << formatIndented("BOOL {}\n", b); }

  std::string ParseTreePrinter::doPrint(const FlowGraphLocation& loc) {
    return formatIndented("at(x{}, y{}, z{}, w{}, h{})\n", loc.x, loc.y, loc.z, loc.width, loc.height);
  }
}  // namespace fluir::debug
