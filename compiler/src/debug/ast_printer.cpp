#include "compiler/debug/ast_printer.hpp"

#include <algorithm>
#include <cassert>
#include <utility>

#include <fmt/format.h>

namespace fluir::debug {
  AstPrinter::AstPrinter(std::ostream& out, bool inOrder) : out_(out), inOrder_(inOrder) { }

  void AstPrinter::print(const ast::AST& ast) {
    // TODO: Sort decls if needed
    for (const auto& decl : ast.declarations) {
      (*this)(decl);
    }
  }

  void AstPrinter::print(const ast::DataFlowGraph& graph) {
    if (inOrder_) {
      doInOrderPrint(graph);
    } else {
      doOutOfOrderPrint(graph);
    }
  }

  void AstPrinter::operator()(const ast::FunctionDecl& func) {
    out_ << formatIndented("Function({}): '{}'\n", func.id, func.name);
    FLUIR_SCOPED_INDENT;
    print(func.statements);
  }

  void AstPrinter::operator()(const ast::BinaryOp& binary) {
    out_ << formatIndented("BinaryOp({}): {}\n", binary.id(), stringify(binary.op()));

    FLUIR_SCOPED_INDENT;
    print(*binary.lhs());
    print(*binary.rhs());
  }

  void AstPrinter::operator()(const ast::UnaryOp& unary) {
    out_ << formatIndented("UnaryOp({}): {}\n", unary.id(), stringify(unary.op()));

    FLUIR_SCOPED_INDENT;
    print(*unary.operand());
  }

  void AstPrinter::operator()(const ast::Constant& constant) {
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

  void AstPrinter::operator()(const ast::Cast& cast) {
    out_ << formatIndented("Cast({}): {} -> {}\n", cast.id(), cast.from(), cast.to());
  }
  void AstPrinter::operator()(const ast::LocalWrite& write) {
    out_ << formatIndented("LocalWrite({})\n", write.id());
    FLUIR_SCOPED_INDENT;
    print(*write.child());
  }
  void AstPrinter::operator()(const ast::LocalRead& read) { out_ << formatIndented("LocalRead({})\n", read.id()); }

  void AstPrinter::doOutOfOrderPrint(const ast::DataFlowGraph& graph) {
    for (const auto& node : graph) {
      print(*node);
    }
  }
  void AstPrinter::doInOrderPrint(const ast::DataFlowGraph& graph) {
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

  void AstPrinter::print(const ast::Node& node) {
    switch (node.kind()) {
      case ast::NodeKind::BinaryOperator:
        return (*this)(*node.as<ast::BinaryOp>());
      case ast::NodeKind::UnaryOperator:
        return (*this)(*node.as<ast::UnaryOp>());
      case ast::NodeKind::Constant:
        return (*this)(*node.as<ast::Constant>());
      case ast::NodeKind::Cast:
        return (*this)(*node.as<ast::Cast>());
      case ast::NodeKind::LocalWrite:
        return (*this)(*node.as<ast::LocalWrite>());
      case ast::NodeKind::LocalRead:
        return (*this)(*node.as<ast::LocalRead>());
    }
  }

}  // namespace fluir::debug
