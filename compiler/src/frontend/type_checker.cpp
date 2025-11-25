#include "compiler/frontend/type_checker.hpp"

#include <cassert>

namespace fluir {
  namespace {
    void checkType(Context& ctx, asg::Constant* constant);
    void checkType(Context& ctx, asg::BinaryOp* binary);
    void checkType(Context& ctx, asg::UnaryOp* unary);

    void checkType(Context& ctx, asg::Node* node) {
      if (node->type() != types::ID_INVALID) {
        // This node has already been type-checked
        return;
      }
      switch (node->kind()) {
        case asg::NodeKind::Constant:
          return checkType(ctx, node->as<asg::Constant>());
        case asg::NodeKind::BinaryOperator:
          return checkType(ctx, node->as<asg::BinaryOp>());
        case asg::NodeKind::UnaryOperator:
          return checkType(ctx, node->as<asg::UnaryOp>());
        default:
          ctx.diagnostics.emitInternalError("Unknown node kind encountered");
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
      checkType(ctx, node.get());
    }

    return decl;
  }

  namespace {
    void checkType(Context& ctx, asg::Constant* constant) {
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
    }

    void checkType(Context& ctx, asg::BinaryOp* binary) {
      checkType(ctx, binary->lhs().get());
      checkType(ctx, binary->rhs().get());

      const auto lhs = binary->lhs()->type();
      const auto rhs = binary->rhs()->type();

      const auto selectedOverload = ctx.symbolTable.selectOverload(lhs, binary->op(), rhs);
      binary->setDefinition(selectedOverload);
      auto [overloadLHS, overloadRHS] = selectedOverload->getParameters();
      if (overloadLHS != lhs) {
        auto castOp = std::make_shared<asg::Cast>(overloadLHS, binary->lhs(), binary->id(), binary->location());
        binary->lhs() = std::move(castOp);
      }
      if (overloadRHS != rhs) {
        auto castOp = std::make_shared<asg::Cast>(overloadRHS, binary->rhs(), binary->id(), binary->location());
        binary->rhs() = std::move(castOp);
      }
    }

    void checkType(Context& ctx, asg::UnaryOp* unary) {
      checkType(ctx, unary->operand().get());

      const auto operand = unary->operand()->type();
      const auto selectedOverload = ctx.symbolTable.selectOverload(unary->op(), operand);
      unary->setDefinition(selectedOverload);
      auto [overloadOp, _] = selectedOverload->getParameters();
      if (overloadOp != operand) {
        auto castOp = std::make_shared<asg::Cast>(overloadOp, unary->operand(), unary->id(), unary->location());
        unary->operand() = std::move(castOp);
      }
    }
  }  // namespace
}  // namespace fluir
