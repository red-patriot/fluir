#include "compiler/frontend/type_checker.hpp"

#include <algorithm>

#include <fmt/format.h>

#include "compiler/utility/results.hpp"
#include "compiler/utility/scope_guard.hpp"

namespace fluir {
  namespace {
    Results<types::TypeID> checkType(Context& ctx, ast::Constant* constant);
    Results<types::TypeID> checkType(Context& ctx, ast::BinaryOp* binary);
    Results<types::TypeID> checkType(Context& ctx, ast::UnaryOp* unary);
    Results<types::TypeID> checkType(Context& ctx, ast::Cast* cast);
    Results<types::TypeID> checkType(Context& ctx, ast::LocalWrite* write);
    Results<types::TypeID> checkType(Context& ctx, ast::LocalRead* read);
    Results<types::TypeID> checkType(Context& ctx, ast::Call* call);
    Results<types::TypeID> checkType(Context& ctx, ast::Conditional* conditional);
    Results<types::TypeID> checkType(Context& ctx,
                                     ast::DataFlowGraph& dfg,
                                     const std::vector<ast::ScopeInput>& inputs,
                                     const std::vector<ID>& outputs);

    bool registerDeclarations(Context& ctx, const ast::AST& ast);

    void insertCast(types::TypeID targetType, ast::UniqueNode& slot, ast::Node* parent) {
      slot = ast::createDependency<ast::Cast>(targetType, std::move(slot), parent->fullId(), parent->location());
      parent->setType(targetType);
    }

    Results<types::TypeID> checkType(Context& ctx, ast::Node* node) {
      if (node->type()) {
        // This node has already been type-checked
        return node->type();
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
        case ast::NodeKind::Call:
          return checkType(ctx, node->as<ast::Call>());
        case ast::NodeKind::Conditional:
          return checkType(ctx, node->as<ast::Conditional>());
      }
      diagnostic::emitInternalError("Unknown node kind encountered");
      return NoResult;
    }
  }  // namespace

  Results<ast::AST> typeCheck(Context& ctx, ast::AST graph) {
    // Pre-pass: register all function signatures before body type-checking
    // so functions can be called regardless of declaration order
    bool failed = !registerDeclarations(ctx, graph);
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
    bool paramsFailed = false;
    for (auto& param : decl.parameters) {
      try {
        auto paramType = ctx.symbolTable.getTypeID(param.typeName);
        if (paramType == types::ID_INVALID) {
          ctx.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_UNRECOGNIZED_TYPE,
                                           ctx.currentFile,
                                           FullID{decl.id, param.id},
                                           "Unrecognized type '{}' for parameter '{}'.",
                                           param.typeName,
                                           param.name);
        }
        paramTypes.push_back(paramType);
        ctx.symbolTable.addLocalVariable(param.id, paramType);
      } catch (const diagnostic::Panic&) {
        paramsFailed = true;
      }
    }
    if (paramsFailed) return NoResult;
    std::optional<types::TypeID> returnType = std::nullopt;
    if (decl.returnValue) {
      returnType = ctx.symbolTable.getTypeID(decl.returnValue->typeName);
      if (returnType == types::ID_INVALID) {
        ctx.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_UNRECOGNIZED_TYPE,
                                         ctx.currentFile,
                                         FullID{decl.id, decl.returnValue->id},
                                         "Unrecognized return type '{}'.",
                                         decl.returnValue->typeName);
      }
    }

    auto funcTypeID = ctx.symbolTable.getFunctionTypeID(decl.name);
    if (funcTypeID == types::ID_INVALID) {
      funcTypeID = ctx.symbolTable.addFunction(decl.name, {paramTypes, returnType});
    }
    decl.type = funcTypeID;

    for (auto& node : decl.statements) {
      if (!checkType(ctx, node.get())) {
        return NoResult;
      }
    }

    // Check return is the right type, or insert a cast if necessary
    if (decl.returnValue) {
      const auto funcType = ctx.symbolTable.getFunctionType(decl.type);
      for (auto& node : decl.statements) {
        if (node->id() != decl.returnValue->id) {
          continue;
        }
        auto* returnNode = node->as<ast::LocalWrite>();
        if (!returnNode) {
          diagnostic::emitInternalError("Unexpected node kind encountered");
        }
        if (returnNode->type() != funcType->returnType().value()) {
          if (!ctx.symbolTable.canImplicitlyConvert(returnNode->type(), funcType->returnType().value())) {
            ctx.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_INCOMPATIBLE_TYPE,
                                             ctx.currentFile,
                                             returnNode->fullId(),
                                             "Cannot implicitly convert '{}' to '{}'.",
                                             ctx.symbolTable.getType(returnNode->type())->name(),
                                             ctx.symbolTable.getType(funcType->returnType().value())->name());
          }
          insertCast(funcType->returnType().value(), returnNode->child(), returnNode);
        }
      }
    }

    return decl;
  }

  namespace {
    bool registerDeclarations(Context& ctx, const ast::AST& ast) {
      bool failed = false;
      for (auto& declaration : ast.declarations) {
        try {
          if (ctx.symbolTable.getFunctionTypeID(declaration.name) != types::ID_INVALID) {
            ctx.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_DUPLICATE_FUNCTION_NAME,
                                             ctx.currentFile,
                                             FullID{declaration.id},
                                             "Function '{}' is already defined.",
                                             declaration.name);
          }
          std::vector<types::TypeID> paramTypes;
          for (const auto& param : declaration.parameters) {
            paramTypes.push_back(ctx.symbolTable.getTypeID(param.typeName));
          }
          std::optional<types::TypeID> returnType;
          if (declaration.returnValue) {
            returnType = ctx.symbolTable.getTypeID(declaration.returnValue->typeName);
          }
          ctx.symbolTable.addFunction(declaration.name, {paramTypes, returnType});
        } catch (const diagnostic::Panic&) {
          failed = true;
        }
      }
      return !failed;
    }

    Results<types::TypeID> checkType(Context&, ast::Constant* constant) {
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
      return constant->type();
    }

    Results<types::TypeID> checkType(Context& ctx, ast::BinaryOp* binary) {
      if (!checkType(ctx, binary->lhs().get()) || !checkType(ctx, binary->rhs().get())) {
        return NoResult;
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
        insertCast(overloadLHS, binary->lhs(), binary);
      }
      if (overloadRHS != rhs) {
        insertCast(overloadRHS, binary->rhs(), binary);
      }
      return binary->type();
    }

    Results<types::TypeID> checkType(Context& ctx, ast::UnaryOp* unary) {
      if (!checkType(ctx, unary->operand().get())) {
        return NoResult;
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
        return NoResult;
      }
      unary->setDefinition(selectedOverload);
      auto [overloadOp, _] = selectedOverload->getParameters();
      if (overloadOp != operand) {
        insertCast(overloadOp, unary->operand(), unary);
      }
      return unary->type();
    }

    Results<types::TypeID> checkType(Context& ctx, ast::Cast* cast) {
      // TODO: Handle user-defined casts here
      return checkType(ctx, cast->operand().get());
    }

    Results<types::TypeID> checkType(Context& ctx, ast::LocalWrite* write) {
      if (!checkType(ctx, write->child().get())) {
        return NoResult;
      }
      auto type = write->child()->type();
      write->setType(type);
      if (write->variable() && ctx.symbolTable.addLocalVariable(write->variable(), type)) {
        return write->type();
      }

      diagnostic::emitInternalError("Encountered an invalid element ID.");
    }

    Results<types::TypeID> checkType(Context& ctx, ast::LocalRead* read) {
      if (read->variable() == INVALID_ID) {
        diagnostic::emitInternalError("Encountered an invalid element ID.");
      }

      auto typeId = ctx.symbolTable.getLocalVariableType(read->variable());
      if (const auto typeDescriptor = ctx.symbolTable.getType(typeId);
          typeDescriptor && typeDescriptor->is<types::Product>()) {
        // To read product types, use the index
        const auto* product = typeDescriptor->as<types::Product>();
        typeId = product->at(read->varIndex());
      }
      read->setType(typeId);
      if (typeId == types::ID_INVALID) {
        ctx.diagnosticSink.emitAtElement(
          diagnostic::Code::ERROR_CANNOT_DETERMINE_TYPE_OF_LOCAL, ctx.currentFile, read->fullId());
      }
      return typeId;
    }

    Results<types::TypeID> checkType(Context& ctx, ast::Call* call) {
      auto* funcType = ctx.symbolTable.getFunctionType(call->target());
      if (!funcType) {
        ctx.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_UNDEFINED_FUNCTION,
                                         ctx.currentFile,
                                         call->fullId(),
                                         "Call to undefined function '{}'.",
                                         call->target());
        return NoResult;
      }
      if (call->arguments().size() != funcType->parameters().size()) {
        ctx.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_WRONG_ARITY,
                                         ctx.currentFile,
                                         call->fullId(),
                                         "Function '{}' expects {} argument(s), but {} were provided.",
                                         call->target(),
                                         funcType->parameters().size(),
                                         call->arguments().size());
        return NoResult;
      }
      bool argsFailed = false;
      for (size_t i = 0; i < call->arguments().size(); ++i) {
        auto& arg = call->arguments()[i];
        try {
          if (!checkType(ctx, arg.get())) {
            argsFailed = true;
            continue;
          }
          const auto argType = arg->type();
          const auto expectedType = funcType->parameters()[i];
          if (argType != expectedType) {
            if (ctx.symbolTable.isMagicBuiltin(funcType)) {
              // Hack: Builtin functions have special internal logic that allows them to accept any input type
              // For now, this check passes magically
              // TODO: Refactor this logic once generics are implemented
              continue;
            }
            if (!ctx.symbolTable.canImplicitlyConvert(argType, expectedType)) {
              ctx.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_INCOMPATIBLE_TYPE,
                                               ctx.currentFile,
                                               arg->fullId(),
                                               "Cannot implicitly convert '{}' to '{}'.",
                                               ctx.symbolTable.getType(argType)->name(),
                                               ctx.symbolTable.getType(expectedType)->name());
            }

            arg = ast::createDependency<ast::Cast>(expectedType, std::move(arg), call->fullId(), call->location());
          }
        } catch (const diagnostic::Panic&) {
          argsFailed = true;
        }
      }
      if (argsFailed) {
        return NoResult;
      }
      if (funcType->returnType()) {
        call->setType(funcType->returnType().value());
      }
      return call->type();
    }

    Results<types::TypeID> checkType(Context& ctx, ast::Conditional* conditional) {
      bool succeeded = true;
      if (!checkType(ctx, conditional->condition().get())) {
        succeeded = false;
      }
      for (auto& input : conditional->inputs()) {
        if (!checkType(ctx, input.node.get())) {
          succeeded = false;
        }
      }
      if (conditional->condition()->type() != types::ID_BOOL) {
        ctx.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_INCOMPATIBLE_TYPE,
                                         ctx.currentFile,
                                         conditional->fullId(),
                                         "Condition requires a BOOL input, found '{}'",
                                         ctx.symbolTable.getType(conditional->condition()->type())->name());
        succeeded = false;
      }

      if (!succeeded) {
        // If the inputs failed, don't check the body
        return NoResult;
      }

      auto thenType = checkType(ctx, conditional->thenBody(), conditional->inputs(), conditional->outputs());
      auto elseType = checkType(ctx, conditional->elseBody(), conditional->inputs(), conditional->outputs());
      if (!thenType && !elseType) {
        return NoResult;
      }

      if (*thenType != *elseType) {
        ctx.diagnosticSink.emitAtElement(diagnostic::Code::ERROR_INCOMPATIBLE_TYPE,
                                         ctx.currentFile,
                                         conditional->fullId(),
                                         "then and else branche outputs are different types, types must be identical");

        return NoResult;
      }

      conditional->setType(*thenType);
      return thenType;
    }

    Results<types::TypeID> checkType(Context& ctx,
                                     ast::DataFlowGraph& dfg,
                                     const std::vector<ast::ScopeInput>& inputs,
                                     const std::vector<ID>& outputs) {
      ctx.symbolTable.pushScope();
      FLUIR_SCOPE_EXIT { ctx.symbolTable.popScope(); };
      // Add all the input types as locals
      std::vector<types::TypeID> inputTypes;
      for (auto& input : inputs) {
        auto inputType = input.node->type();
        inputTypes.push_back(inputType);
        ctx.symbolTable.addLocalVariable(input.innerId, inputType);
      }

      bool succeeded = true;
      for (auto& node : dfg) {
        if (!checkType(ctx, node.get())) {
          succeeded = false;
        }
      }

      std::vector<types::TypeID> outputTypes;
      for (auto& output : outputs) {
        auto type = ctx.symbolTable.getLocalVariableType(output);
        if (type == types::ID_INVALID) {
          succeeded = false;
        }
        outputTypes.push_back(type);
      }
      if (succeeded) {
        if (outputTypes.size() == 1) {
          // If there is one output, the type is just that output type
          return outputTypes.front();
        } else {
          return ctx.symbolTable.addType(types::Product::anonymous(std::move(outputTypes)));
        }
      }

      return NoResult;
    }

  }  // namespace
}  // namespace fluir
