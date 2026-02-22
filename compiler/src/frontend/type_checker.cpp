#include "compiler/frontend/type_checker.hpp"

#include <compiler/utility/scope_guard.hpp>
#include <fmt/format.h>

namespace fluir {
  namespace {
    bool checkType(Context& ctx, ast::Constant* constant);
    bool checkType(Context& ctx, ast::BinaryOp* binary);
    bool checkType(Context& ctx, ast::UnaryOp* unary);
    bool checkType(Context& ctx, ast::Cast* cast);
    bool checkType(Context& ctx, ast::LocalWrite* write);
    bool checkType(Context& ctx, ast::LocalRead* read);

    bool checkType(Context& ctx, ast::Node* node) {
      if (node->type() != types::ID_INVALID) {
        // This node has already been type-checked
        return true;
      }
      switch (node->kind()) {
        case ast::NodeKind::Constant:
          return checkType(ctx, node->as<ast::Constant>());
        case ast::NodeKind::BinaryOperator:
          return checkType(ctx, node->as<ast::BinaryOp>());
        case ast::NodeKind::UnaryOperator:
          return checkType(ctx, node->as<ast::UnaryOp>());
        case ast::NodeKind::Cast:
          return checkType(ctx, node->as<ast::Cast>());
        case ast::NodeKind::LocalWrite:
          return checkType(ctx, node->as<ast::LocalWrite>());
        case ast::NodeKind::LocalRead:
          return checkType(ctx, node->as<ast::LocalRead>());
        default:
          diagnostic::emitInternalError("Unknown node kind encountered");
      }
    }
  }  // namespace

  Results<ast::AST> typeCheck(Context& ctx, ast::AST graph) {
    bool failed = false;
    for (auto& declaration : graph.declarations) {
      try {
        auto result = checkDeclType(ctx, std::move(declaration));
        if (!result.has_value()) {
          failed = true;
          continue;
        }
        declaration = std::move(result.value());
      } catch (const diagnostic::Panic&) {
        failed = true;
      }
    }
    return failed ? NoResult : std::make_optional(std::move(graph));
  }

  Results<ast::Declaration> checkDeclType(Context& ctx, ast::Declaration decl) {
    ctx.symbolTable.pushScope();
    FLUIR_SCOPE_EXIT { ctx.symbolTable.popScope(); };
    // Add all the function's parameter types as locals
    std::vector<types::TypeID> paramTypes;
    for (auto& param : decl.parameters) {
      auto paramType = ctx.symbolTable.getTypeID(param.typeName);
      param.type = paramType;
      paramTypes.push_back(paramType);
      // TODO: Check for invalid types
      ctx.symbolTable.addLocalVariable(param.id, paramType);
    }
    std::optional<types::TypeID> returnType = std::nullopt;
    if (decl.returnValue) {
      returnType = ctx.symbolTable.getTypeID(decl.returnValue->typeName);
    }

    auto funcType = ctx.symbolTable.addFunction(decl.name, {paramTypes, returnType});
    decl.type = funcType;

    for (auto& node : decl.statements) {
      if (!checkType(ctx, node.get())) {
        return NoResult;
      }
    }

    return decl;
  }

  namespace {
    bool checkType(Context&, ast::Constant* constant) {
      // This is dependent on the order of the types in Literal
      // TODO: Refactor this to be independent
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
          diagnostic::emitInternalError("Entered an impossible case");
          break;
      }
      return true;
    }

    bool checkType(Context& ctx, ast::BinaryOp* binary) {
      if (!checkType(ctx, binary->lhs().get()) || !checkType(ctx, binary->rhs().get())) {
        return false;
      }

      const auto lhs = binary->lhs()->type();
      const auto rhs = binary->rhs()->type();

      const auto selectedOverload = ctx.symbolTable.selectOverload(lhs, binary->op(), rhs);
      if (!selectedOverload) {
        ctx.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_OPERATOR_OVERLOAD_RESOLUTION_FAILED,
                                         ctx.currentFile,
                                         binary->fullId(),
                                         "No binary {} exists with operand types {}, {}.",
                                         stringify(binary->op()),
                                         ctx.symbolTable.getType(lhs)->name(),
                                         ctx.symbolTable.getType(rhs)->name());
      }
      binary->setDefinition(selectedOverload);
      auto [overloadLHS, overloadRHS] = selectedOverload->getParameters();
      if (overloadLHS != lhs) {
        binary->lhs() =
          ast::createDependency<ast::Cast>(overloadLHS, std::move(binary->lhs()), binary->fullId(), binary->location());
      }
      if (overloadRHS != rhs) {
        binary->rhs() =
          ast::createDependency<ast::Cast>(overloadRHS, std::move(binary->rhs()), binary->fullId(), binary->location());
      }
      return true;
    }

    bool checkType(Context& ctx, ast::UnaryOp* unary) {
      if (!checkType(ctx, unary->operand().get())) {
        return false;
      }

      const auto operand = unary->operand()->type();
      const auto selectedOverload = ctx.symbolTable.selectOverload(unary->op(), operand);
      if (!selectedOverload) {
        ctx.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_OPERATOR_OVERLOAD_RESOLUTION_FAILED,
                                         ctx.currentFile,
                                         unary->fullId(),
                                         "No unary {} exists with operand type {}.",
                                         stringify(unary->op()),
                                         ctx.symbolTable.getType(operand)->name());
        return false;
      }
      unary->setDefinition(selectedOverload);
      auto [overloadOp, _] = selectedOverload->getParameters();
      if (overloadOp != operand) {
        unary->operand() =
          ast::createDependency<ast::Cast>(overloadOp, std::move(unary->operand()), unary->fullId(), unary->location());
      }
      return true;
    }

    bool checkType(Context& ctx, ast::Cast* cast) {
      // TODO: Handle user-defined casts here
      return checkType(ctx, cast->operand().get());
    }

    bool checkType(Context& ctx, ast::LocalWrite* write) {
      if (!checkType(ctx, write->child().get())) {
        return false;
      }
      auto type = write->child()->type();
      write->setType(type);
      if (write->variable() != INVALID_ID && ctx.symbolTable.addLocalVariable(write->variable(), type)) {
        return true;
      }

      diagnostic::emitInternalError("Encountered an invalid element ID.");
    }

    bool checkType(Context& ctx, ast::LocalRead* read) {
      if (read->variable() == INVALID_ID) {
        diagnostic::emitInternalError("Encountered an invalid element ID.");
      }

      auto type = ctx.symbolTable.getLocalVariableType(read->variable());
      read->setType(type);
      if (type == types::ID_INVALID) {
        ctx.diagnosticSink.emitAtElement(
          diagnostic::Code::ERROR_CANNOT_DETERMINE_TYPE_OF_LOCAL, ctx.currentFile, read->fullId());
      }
      return true;
    }

  }  // namespace
}  // namespace fluir
