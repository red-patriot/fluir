#include "compiler/debug/asg_printer.hpp"

#include <algorithm>
#include <utility>

#include <fmt/format.h>

namespace fluir::debug {
  AsgPrinter::AsgPrinter(std::ostream& out, bool inOrder) : out_(out), inOrder_(inOrder) { }

  void AsgPrinter::print(const asg::ASG& asg) {
    // TODO: Sort decls if needed
    for (const auto& decl : asg.declarations) {
      (*this)(decl);
    }
  }

  void AsgPrinter::print(const asg::DataFlowGraph& graph) {
    if (inOrder_) {
      doInOrderPrint(graph);
    } else {
      doOutOfOrderPrint(graph);
    }
  }

  void AsgPrinter::operator()(const asg::FunctionDecl& func) {
    out_ << formatIndented("Function({}): '{}'\n", func.id, func.name);
    [[maybe_unused]] auto _ = indent();
    print(func.statements);
  }

  void AsgPrinter::operator()(const asg::BinaryOp& binary) {
    out_ << formatIndented("BinaryOp({}): {}\n", binary.id(), stringify(binary.op()));

    [[maybe_unused]] auto _ = indent();
    print(*binary.lhs());
    print(*binary.rhs());
  }

  void AsgPrinter::operator()(const asg::UnaryOp& unary) {
    out_ << formatIndented("UnaryOp({}): {}\n", unary.id(), stringify(unary.op()));

    [[maybe_unused]] auto _ = indent();
    print(*unary.operand());
  }

  void AsgPrinter::operator()(const asg::Constant& constant) {
    using namespace literals_types;
    // TODO: Use type information instead of hardcoding this here
    auto printer = [this, &constant]<typename T>(const T& val) {
      if constexpr (std::is_same_v<T, F64>) {
        out_ << formatIndented("ConstantF64({}): {:.4f}\n", constant.id(), val);
      } else if constexpr (std::is_same_v<T, I8>) {
        out_ << formatIndented("ConstantI8({}): {}\n", constant.id(), val);
      } else if constexpr (std::is_same_v<T, I16>) {
        out_ << formatIndented("ConstantI16({}): {}\n", constant.id(), val);
      } else if constexpr (std::is_same_v<T, I32>) {
        out_ << formatIndented("ConstantI32({}): {}\n", constant.id(), val);
      } else if constexpr (std::is_same_v<T, I64>) {
        out_ << formatIndented("ConstantI64({}): {}\n", constant.id(), val);
      } else if constexpr (std::is_same_v<T, U8>) {
        out_ << formatIndented("ConstantU8({}): {}\n", constant.id(), val);
      } else if constexpr (std::is_same_v<T, U16>) {
        out_ << formatIndented("ConstantU16({}): {}\n", constant.id(), val);
      } else if constexpr (std::is_same_v<T, U32>) {
        out_ << formatIndented("ConstantU32({}): {}\n", constant.id(), val);
      } else if constexpr (std::is_same_v<T, U64>) {
        out_ << formatIndented("ConstantU64({}): {}\n", constant.id(), val);
      }
    };
    std::visit(printer, constant.value());
  }

  void AsgPrinter::doOutOfOrderPrint(const asg::DataFlowGraph& graph) {
    for (const auto& node : graph) {
      print(*node);
    }
  }
  void AsgPrinter::doInOrderPrint(const asg::DataFlowGraph& graph) {
    // TODO: Sort elements if needed
    std::vector<std::pair<ID, size_t>> idIndices;
    idIndices.reserve(graph.size());
    for (size_t i = 0; i != graph.size(); ++i) {
      idIndices.emplace_back(std::pair{graph.at(i)->id(), i});
    }

    std::ranges::sort(idIndices, [](const std::pair<ID, size_t>& lhs, const std::pair<ID, size_t>& rhs) {
      return lhs.first < rhs.first;
    });

    for (const auto& [id, index] : idIndices) {
      auto& node = graph.at(index);
      print(*node);
    }
  }

  void AsgPrinter::print(const asg::Node& node) {
    switch (node.kind()) {
      case asg::NodeKind::BinaryOperator:
        return (*this)(*node.as<asg::BinaryOp>());
      case asg::NodeKind::UnaryOperator:
        return (*this)(*node.as<asg::UnaryOp>());
      case asg::NodeKind::Constant:
        return (*this)(*node.as<asg::Constant>());
    }
  }

}  // namespace fluir::debug
