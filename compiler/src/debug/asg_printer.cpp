module;

#include <algorithm>
#include <utility>
#include <variant>
#include <vector>

#include <fmt/format.h>

module fluir.debug.asg_printer;
import fluir.models.id;
import fluir.models.location;
import fluir.models.operators;

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
    out_ << formatIndented("BinaryOp({}): {}\n", binary.id, stringify(binary.op));

    [[maybe_unused]] auto _ = indent();
    std::visit(*this, *binary.lhs);
    std::visit(*this, *binary.rhs);
  }

  void AsgPrinter::operator()(const asg::UnaryOp& unary) {
    out_ << formatIndented("UnaryOp({}): {}\n", unary.id, stringify(unary.op));

    [[maybe_unused]] auto _ = indent();
    std::visit(*this, *unary.operand);
  }

  void AsgPrinter::operator()(const asg::ConstantFP& constant) {
    out_ << formatIndented("ConstantFP({}): {:.4f}\n", constant.id, constant.value);
  }

  void AsgPrinter::doOutOfOrderPrint(const asg::DataFlowGraph& graph) {
    for (const auto& node : graph) {
      std::visit(*this, node);
    }
  }
  void AsgPrinter::doInOrderPrint(const asg::DataFlowGraph& graph) {
    // TODO: Sort elements if needed
    std::vector<std::pair<ID, size_t>> idIndices;
    idIndices.reserve(graph.size());
    for (size_t i = 0; i != graph.size(); ++i) {
      idIndices.emplace_back(std::pair{graph.at(i).id(), i});
    }

    std::ranges::sort(idIndices, [](const std::pair<ID, size_t>& lhs, const std::pair<ID, size_t>& rhs) {
      return lhs.first < rhs.first;
    });

    for (const auto& [id, index] : idIndices) {
      auto& node = graph.at(index);
      std::visit(*this, node);
    }
  }
}  // namespace fluir::debug
