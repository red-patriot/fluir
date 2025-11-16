#include "compiler/frontend/type_checker.hpp"

#include <cassert>

namespace fluir {
  namespace {
    void checkType(Context& ctx, asg::Constant* constant);
    void checkType(Context& ctx, asg::BinaryOp* binary);
    void checkType(Context& ctx, asg::UnaryOp* unary);

    void checkType(Context& ctx, asg::Node* node) {
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

  Results<asg::ASG> checkTypes(Context& ctx, asg::ASG graph) {
    for (auto i = graph.declarations.begin(); i != graph.declarations.end(); ++i) {
      auto result = checkTypes(ctx, std::move(*i));
      if (!result.has_value()) {
        return NoResult;
      }
      *i = std::move(result.value());
    }
    return graph;
  }

  Results<asg::Declaration> checkTypes(Context& ctx, asg::Declaration decl) {
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
          constant->setType(ctx.symbolTable.getType("F64"));
          break;
        case 1:  // I8
          constant->setType(ctx.symbolTable.getType("I8"));
          break;
        case 2:  // I16
          constant->setType(ctx.symbolTable.getType("I16"));
          break;
        case 3:  // I32
          constant->setType(ctx.symbolTable.getType("I32"));
          break;
        case 4:  // I64
          constant->setType(ctx.symbolTable.getType("I64"));
          break;
        case 5:  // U8
          constant->setType(ctx.symbolTable.getType("U8"));
          break;
        case 6:  // U16
          constant->setType(ctx.symbolTable.getType("U16"));
          break;
        case 7:  // U32
          constant->setType(ctx.symbolTable.getType("U32"));
          break;
        case 8:  // U64
          constant->setType(ctx.symbolTable.getType("U64"));
          break;
        default:
          ctx.diagnostics.emitInternalError("Entered an impossible case");
          break;
      }
    }

    void checkType(Context& ctx, asg::BinaryOp* binary) {
      // TODO: CHECK FOR NULLS
      // TODO: CHECK FOR SHARING
      checkType(ctx, binary->lhs().get());
      checkType(ctx, binary->rhs().get());

      const auto lhs = binary->lhs()->type();
      const auto rhs = binary->rhs()->type();

      const auto selectedOverload = ctx.symbolTable.selectOverload(lhs, binary->op(), rhs);
      binary->setDefinition(selectedOverload);
    }

    void checkType(Context& ctx, asg::UnaryOp* unary) {
      // TODO: CHECK FOR NULLS
      // TODO: CHECK FOR SHARING
      checkType(ctx, unary->operand().get());

      const auto operand = unary->operand()->type();
      const auto selectedOverload = ctx.symbolTable.selectOverload(unary->op(), operand);
      unary->setDefinition(selectedOverload);
    }
  }  // namespace
}  // namespace fluir
