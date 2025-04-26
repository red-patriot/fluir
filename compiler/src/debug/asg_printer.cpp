#include "compiler/debug/asg_printer.hpp"

#include <algorithm>
#include <utility>

#include <fmt/format.h>

namespace fluir::debug {
  AsgPrinter::AsgPrinter(std::ostream& out, bool inOrder) :
      out_(out),
      inOrder_(inOrder) {
  }

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
    out_ << indent() << fmt::format("Function({}): '{}'", func.id, func.name) << '\n';
    indent_ += 2;
    print(func.statements);
    indent_ -= 2;
  }

  void AsgPrinter::operator()(const asg::BinaryOp& binary) {
    out_ << indent() << fmt::format("BinaryOp({}): {}", binary.id, stringify(binary.op)) << '\n';

    indent_ += 2;
    std::visit(*this, *binary.lhs);
    std::visit(*this, *binary.rhs);
    indent_ -= 2;
  }

  void AsgPrinter::operator()(const asg::UnaryOp& unary) {
    out_ << indent() << fmt::format("UnaryOp({}): {}", unary.id, stringify(unary.op)) << '\n';

    indent_ += 2;
    std::visit(*this, *unary.operand);
    indent_ -= 2;
  }

  void AsgPrinter::operator()(const asg::ConstantFP& constant) {
    out_ << indent() << fmt::format("ConstantFP({}): {:.4f}", constant.id, constant.value) << '\n';
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

    std::ranges::sort(idIndices,
                      [](const std::pair<ID, size_t>& lhs, const std::pair<ID, size_t>& rhs) {
                        return lhs.first < rhs.first;
                      });

    for (const auto& [id, index] : idIndices) {
      auto& node = graph.at(index);
      std::visit(*this, node);
    }
  }

  std::string AsgPrinter::indent() {
    return std::string(indent_, ' ');
  }
}  // namespace fluir::debug
