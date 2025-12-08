#include "compiler/frontend/type_checker.hpp"

#include <fmt/format.h>

#include "compiler/models/asg/diagnostic_location.hpp"

namespace fluir {
  namespace {
    bool checkType(Context& ctx, asg::Constant* constant);
    bool checkType(Context& ctx, asg::BinaryOp* binary);
    bool checkType(Context& ctx, asg::UnaryOp* unary);
    bool checkType(Context& ctx, asg::Cast* cast);

    bool checkType(Context& ctx, asg::Node* node) {
      if (node->type() != types::ID_INVALID) {
        // This node has already been type-checked
        return true;
      }
      switch (node->kind()) {
        case asg::NodeKind::Constant:
          return checkType(ctx, node->as<asg::Constant>());
        case asg::NodeKind::BinaryOperator:
          return checkType(ctx, node->as<asg::BinaryOp>());
        case asg::NodeKind::UnaryOperator:
          return checkType(ctx, node->as<asg::UnaryOp>());
        case asg::NodeKind::Cast:
          return checkType(ctx, node->as<asg::Cast>());
        default:
          ctx.diagnostics.emitInternalError("Unknown node kind encountered");
          return false;
      }
    }
  }  // namespace

  Results<asg::ASG> typeCheck(Context& ctx, asg::ASG graph) {
    for (auto& declaration : graph.declarations) {
      auto result = checkDeclType(ctx, std::move(declaration));
      if (!result.has_value()) {
        return NoResult;
      }
      declaration = std::move(result.value());
    }
    return graph;
  }

  Results<asg::Declaration> checkDeclType(Context& ctx, asg::Declaration decl) {
    for (auto& node : decl.statements) {
      if (!checkType(ctx, node.get())) {
        return NoResult;
      }
    }

    return decl;
  }

  namespace {
    bool checkType(Context& ctx, asg::Constant* constant) {
      // This is dependent on the order of the types in Literal
      switch (constant->value().index()) {
        case 0:  // F64
          constant->setType(types::ID_F64);
          break;
        case 1:  // I8
          constant->setType(types::ID_I8);
          break;
        case 2:  // I16
          constant->setType(types::ID_I16);
          break;
        case 3:  // I32
          constant->setType(types::ID_I32);
          break;
        case 4:  // I64
          constant->setType(types::ID_I64);
          break;
        case 5:  // U8
          constant->setType(types::ID_U8);
          break;
        case 6:  // U16
          constant->setType(types::ID_U16);
          break;
        case 7:  // U32
          constant->setType(types::ID_U32);
          break;
        case 8:  // U64
          constant->setType(types::ID_U64);
          break;
        default:
          ctx.diagnostics.emitInternalError("Entered an impossible case");
          break;
      }
      return true;
    }

    bool checkType(Context& ctx, asg::BinaryOp* binary) {
      if (!checkType(ctx, binary->lhs().get()) || !checkType(ctx, binary->rhs().get())) {
        return false;
      }

      const auto lhs = binary->lhs()->type();
      const auto rhs = binary->rhs()->type();

      const auto selectedOverload = ctx.symbolTable.selectOverload(lhs, binary->op(), rhs);
      if (!selectedOverload) {
        ctx.diagnostics.emitError(
          fmt::format("No suitable operator candidates for binary {} with operand types {}, {}.",
                      stringify(binary->op()),
                      ctx.symbolTable.getType(lhs)->name(),
                      ctx.symbolTable.getType(rhs)->name()),
          std::make_shared<asg::DiagnosticLocation>(ctx.currentFile.filename().string(), binary));
        return false;
      }
      binary->setDefinition(selectedOverload);
      auto [overloadLHS, overloadRHS] = selectedOverload->getParameters();
      if (overloadLHS != lhs) {
        auto castOp = std::make_shared<asg::Cast>(overloadLHS, binary->lhs(), binary->fullId(), binary->location());
        binary->lhs() = std::move(castOp);
      }
      if (overloadRHS != rhs) {
        auto castOp = std::make_shared<asg::Cast>(overloadRHS, binary->rhs(), binary->fullId(), binary->location());
        binary->rhs() = std::move(castOp);
      }
      return true;
    }

    bool checkType(Context& ctx, asg::UnaryOp* unary) {
      if (!checkType(ctx, unary->operand().get())) {
        return false;
      }

      const auto operand = unary->operand()->type();
      const auto selectedOverload = ctx.symbolTable.selectOverload(unary->op(), operand);
      if (!selectedOverload) {
        ctx.diagnostics.emitError(
          fmt::format("No suitable operator candidates for unary {} with operand type {}.",
                      stringify(unary->op()),
                      ctx.symbolTable.getType(operand)->name()),
          std::make_shared<asg::DiagnosticLocation>(ctx.currentFile.filename().string(), unary));
        return false;
      }
      unary->setDefinition(selectedOverload);
      auto [overloadOp, _] = selectedOverload->getParameters();
      if (overloadOp != operand) {
        auto castOp = std::make_shared<asg::Cast>(overloadOp, unary->operand(), unary->fullId(), unary->location());
        unary->operand() = std::move(castOp);
      }
      return true;
    }

    bool checkType(Context& ctx, asg::Cast* cast) {
      // TODO: Handle user-defined casts here
      return checkType(ctx, cast->operand().get());
    }
  }  // namespace
}  // namespace fluir
