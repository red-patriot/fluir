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
    [[maybe_unused]] auto _ = indent();
    out_ << formatIndented("FunctionDecl({})\n", func.name) << doPrint(func.location);

    {
      out_ << formatIndented("body\n");
      auto orderedNodes = keyOrder(func.body.nodes);
      [[maybe_unused]] auto _ = indent();
      for (const auto& node : orderedNodes) {
        std::visit(*this, func.body.nodes.at(node));
      }
    }

    {
      out_ << formatIndented("conduits\n");
      auto orderedConduits = keyOrder(func.body.conduits);
      [[maybe_unused]] auto _ = indent();
      for (const auto& conduit : orderedConduits) {
        (*this)(func.body.conduits.at(conduit));
      }
    }
  }

  void ParseTreePrinter::operator()(const pt::Binary& binary) {
    out_ << formatIndented("{}:\n", binary.id);
    [[maybe_unused]] auto _ = indent();
    out_ << formatIndented("Binary\n") << doPrint(binary.location) << formatIndented("{}\n", stringify(binary.op))
         << formatIndented("lhs{}\n", binary.lhs) << formatIndented("rhs{}\n", binary.rhs);
  }
  void ParseTreePrinter::operator()(const pt::Unary& unary) {
    out_ << formatIndented("{}:\n", unary.id);
    [[maybe_unused]] auto _ = indent();
    out_ << formatIndented("Unary\n") << doPrint(unary.location) << formatIndented("{}\n", stringify(unary.op))
         << formatIndented("lhs{}\n", unary.lhs);
  }
  void ParseTreePrinter::operator()(const pt::Constant& constant) {
    out_ << formatIndented("{}:\n", constant.id);
    [[maybe_unused]] auto _ = indent();
    out_ << formatIndented("Constant\n") << doPrint(constant.location) << formatIndented("F64 {}\n", constant.value);
  }

  void ParseTreePrinter::operator()(const pt::Conduit& conduit) {
    out_ << formatIndented("{}:\n", conduit.id);
    [[maybe_unused]] auto _ = indent();
    out_ << formatIndented("Conduit\n") << formatIndented("in{}.{}\n", conduit.input, conduit.index)
         << formatIndented("children\n");
    {
      [[maybe_unused]] auto _ = indent();
      for (const auto& child : conduit.children) {
        out_ << formatIndented("out{}.{}\n", child.target, child.index);
      }
    }
  }

  std::string ParseTreePrinter::doPrint(const FlowGraphLocation& loc) {
    return formatIndented("at(x{}, y{}, z{}, w{}, h{})\n", loc.x, loc.y, loc.z, loc.width, loc.height);
  }
}  // namespace fluir::debug
