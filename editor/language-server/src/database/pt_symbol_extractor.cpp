#include "lsp/database/pt_symbol_extractor.hpp"

#include <algorithm>

#include <compiler/models/operator.hpp>
#include <compiler/types/builtin_symbols.hpp>
#include <compiler/types/symbol_table.hpp>
#include <compiler/types/typeid.hpp>
#include <compiler/utility/diagnostic/code.hpp>
#include <compiler/utility/diagnostic/pretty_msg.hpp>
#include <fmt/format.h>

namespace fluir::lsp {

  namespace {
    // There be dragons here...
    template <class... Ts>
    struct overloaded : Ts... {
      using Ts::operator()...;
    };

    constexpr std::string_view TYPE_NAMES[] = {"F64", "I8", "I16", "I32", "I64", "U8", "U16", "U32", "U64"};

    std::string literalTypeName(const pt::Literal& lit) { return std::string{TYPE_NAMES[lit.index()]}; }

    // TODO: Fix this hack
    types::TypeID literalTypeID(const pt::Literal& lit) { return static_cast<types::TypeID>(lit.index() + 1); }

    void emitDiag(std::vector<api::ModuleDiagnostic>& diagnostics,
                  FullID location,
                  diagnostic::Code code,
                  const std::string& extra) {
      auto msg =
        extra.empty() ? diagnostic::prettyMessage(code) : fmt::format("{}: {}", diagnostic::prettyMessage(code), extra);
      auto severity = diagnostic::isError(code) ? api::DiagnosticSeverity::ERROR : api::DiagnosticSeverity::WARNING;
      diagnostics.push_back(
        api::ModuleDiagnostic{.location = std::move(location), .severity = severity, .message = std::move(msg)});
    }

    // Build a map: targetNodeID -> { inputIndex -> sourceNodeID }
    // This tells us which node feeds into which input slot of another node.
    std::unordered_map<ID, std::unordered_map<int, ID>> buildInputMap(const pt::Block& block) {
      std::unordered_map<ID, std::unordered_map<int, ID>> result;
      for (const auto& [_, conduit] : block.conduits) {
        for (const auto& output : conduit.children) {
          result[output.target][output.index] = conduit.input;
        }
      }
      return result;
    }

  }  // namespace

  ExtractionResult extractSymbols(const pt::ParseTree& tree, [[maybe_unused]] const std::filesystem::path& file) {
    ExtractionResult result;
    auto symbolTable = types::buildSymbolTable();

    // Phase 1: Register declarations and detect duplicates
    std::unordered_map<std::string, const pt::FunctionDecl*> funcByName;

    for (const auto& [declId, decl] : tree.declarations) {
      const auto& func = std::get<pt::FunctionDecl>(decl);

      if (funcByName.contains(func.name)) {
        emitDiag(result.diagnostics,
                 FullID{declId},
                 diagnostic::Code::ERROR_DUPLICATE_FUNCTION_NAME,
                 fmt::format("Function '{}' is already defined.", func.name));
      } else {
        funcByName[func.name] = &func;
      }
    }

    // Phase 2: Extract symbols per declaration
    for (const auto& [declId, decl] : tree.declarations) {
      const auto& func = std::get<pt::FunctionDecl>(decl);
      DeclarationInfo info;

      // Function symbol
      api::Symbol funcSymbol;
      funcSymbol.name = "func " + func.name;

      if (func.input) {
        std::vector<std::string> inTypes;
        for (const auto& param : func.input->parameters) {
          inTypes.push_back(param.typeName);
        }
        if (!inTypes.empty()) {
          funcSymbol.inType = std::move(inTypes);
        }
      }

      if (func.output && func.output->ret) {
        funcSymbol.outType = func.output->ret->typeName;
      }

      info.symbol = std::move(funcSymbol);

      // Parameter symbols
      if (func.input) {
        for (const auto& param : func.input->parameters) {
          api::Symbol paramSymbol;
          paramSymbol.name = param.typeName + " Parameter";

          auto typeId = symbolTable.getTypeID(param.typeName);
          if (typeId == types::ID_INVALID) {
            emitDiag(result.diagnostics,
                     FullID{declId, param.id},
                     diagnostic::Code::ERROR_UNRECOGNIZED_TYPE,
                     fmt::format("Unrecognized type '{}' for parameter '{}'.", param.typeName, param.name));
          }

          info.symbols[param.id] = std::move(paramSymbol);
        }
      }

      // Return symbol
      if (func.output && func.output->ret) {
        const auto& ret = *func.output->ret;
        api::Symbol retSymbol;
        retSymbol.name = ret.typeName + " Return";

        auto typeId = symbolTable.getTypeID(ret.typeName);
        if (typeId == types::ID_INVALID) {
          emitDiag(result.diagnostics,
                   FullID{declId, ret.id},
                   diagnostic::Code::ERROR_UNRECOGNIZED_TYPE,
                   fmt::format("Unrecognized return type '{}'.", ret.typeName));
        }

        info.symbols[ret.id] = std::move(retSymbol);
      }

      // Body node symbols
      for (const auto& [nodeId, node] : func.body.nodes) {
        api::Symbol nodeSymbol;

        std::visit(overloaded{
                     [&](const pt::Constant& c) {
                       auto typeName = literalTypeName(c.value);
                       nodeSymbol.name = typeName + " Constant";
                       nodeSymbol.outType = typeName;
                     },
                     [&](const pt::Binary& b) { nodeSymbol.name = fmt::format("binary {}", stringify(b.op)); },
                     [&](const pt::Unary& u) { nodeSymbol.name = fmt::format("unary {}", stringify(u.op)); },
                     [&](const pt::Call& call) {
                       auto it = funcByName.find(call.target);
                       if (it == funcByName.end()) {
                         // Undefined function - just use the target name
                         nodeSymbol.name = call.target;
                         emitDiag(result.diagnostics,
                                  FullID{declId, nodeId},
                                  diagnostic::Code::ERROR_UNDEFINED_FUNCTION,
                                  fmt::format("Call to undefined function '{}'.", call.target));
                       } else {
                         const auto* targetFunc = it->second;
                         nodeSymbol.name = call.target;

                         // Build inType/outType from target function's declaration
                         if (targetFunc->input) {
                           std::vector<std::string> inTypes;
                           for (const auto& param : targetFunc->input->parameters) {
                             inTypes.push_back(param.typeName);
                           }
                           if (!inTypes.empty()) {
                             nodeSymbol.inType = std::move(inTypes);
                           }
                         }
                         if (targetFunc->output && targetFunc->output->ret) {
                           nodeSymbol.outType = targetFunc->output->ret->typeName;
                         }

                         // Validate argument count
                         auto expectedArgs =
                           targetFunc->input ? static_cast<int>(targetFunc->input->parameters.size()) : 0;
                         auto actualArgs = static_cast<int>(call.arguments.size());
                         if (actualArgs != expectedArgs) {
                           emitDiag(result.diagnostics,
                                    FullID{declId, nodeId},
                                    diagnostic::Code::ERROR_WRONG_ARITY,
                                    fmt::format("Function '{}' expects {} argument(s), but {} were provided.",
                                                call.target,
                                                expectedArgs,
                                                actualArgs));
                         }
                       }
                     },
                   },
                   node);

        info.symbols[nodeId] = std::move(nodeSymbol);
      }

      result.declarations[declId] = std::move(info);
    }

    // Phase 3: Type propagation and validation
    for (const auto& [declId, decl] : tree.declarations) {
      const auto& func = std::get<pt::FunctionDecl>(decl);

      // Seed the type map
      std::unordered_map<ID, types::TypeID> typeMap;

      if (func.input) {
        for (const auto& param : func.input->parameters) {
          auto typeId = symbolTable.getTypeID(param.typeName);
          if (typeId != types::ID_INVALID) {
            typeMap[param.id] = typeId;
          }
        }
      }

      for (const auto& [nodeId, node] : func.body.nodes) {
        if (auto* c = std::get_if<pt::Constant>(&node)) {
          typeMap[nodeId] = literalTypeID(c->value);
        }
      }

      auto inputMap = buildInputMap(func.body);

      // Resolve types using fixed-point iteration (handles dependencies between operator nodes)
      bool changed = true;
      while (changed) {
        changed = false;
        for (const auto& [nodeId, node] : func.body.nodes) {
          if (typeMap.contains(nodeId)) continue;  // already resolved

          std::visit(overloaded{
                       [&](const pt::Binary& b) {
                         auto inMapIt = inputMap.find(nodeId);
                         if (inMapIt == inputMap.end()) return;

                         const auto& inputs = inMapIt->second;
                         auto lhsIt = inputs.find(0);
                         auto rhsIt = inputs.find(1);
                         if (lhsIt == inputs.end() || rhsIt == inputs.end()) return;

                         auto lhsTypeIt = typeMap.find(lhsIt->second);
                         auto rhsTypeIt = typeMap.find(rhsIt->second);
                         if (lhsTypeIt == typeMap.end() || rhsTypeIt == typeMap.end()) return;  // not ready yet

                         auto* overload = symbolTable.selectOverload(lhsTypeIt->second, b.op, rhsTypeIt->second);
                         if (overload != nullptr) {
                           typeMap[nodeId] = overload->getReturn();
                           changed = true;
                         }
                       },
                       [&](const pt::Unary& u) {
                         auto inMapIt = inputMap.find(nodeId);
                         if (inMapIt == inputMap.end()) return;

                         const auto& inputs = inMapIt->second;
                         auto operandIt = inputs.find(0);
                         if (operandIt == inputs.end()) return;

                         auto operandTypeIt = typeMap.find(operandIt->second);
                         if (operandTypeIt == typeMap.end()) return;  // not ready yet

                         auto* overload = symbolTable.selectOverload(u.op, operandTypeIt->second);
                         if (overload != nullptr) {
                           typeMap[nodeId] = overload->getReturn();
                           changed = true;
                         }
                       },
                       [&](const pt::Call& call) {
                         auto it = funcByName.find(call.target);
                         if (it == funcByName.end()) return;

                         const auto* targetFunc = it->second;
                         if (targetFunc->output && targetFunc->output->ret) {
                           auto retTypeId = symbolTable.getTypeID(targetFunc->output->ret->typeName);
                           if (retTypeId != types::ID_INVALID) {
                             typeMap[nodeId] = retTypeId;
                             changed = true;
                           }
                         }
                       },
                       [&](const pt::Constant&) { /* already handled */ },
                     },
                     node);
        }
      }

      // Emit diagnostics for nodes that couldn't be resolved
      for (const auto& [nodeId, node] : func.body.nodes) {
        if (typeMap.contains(nodeId)) continue;

        std::visit(overloaded{
                     [&](const pt::Binary& b) {
                       auto inMapIt = inputMap.find(nodeId);
                       if (inMapIt == inputMap.end()) return;

                       const auto& inputs = inMapIt->second;
                       auto lhsIt = inputs.find(0);
                       auto rhsIt = inputs.find(1);
                       if (lhsIt == inputs.end() || rhsIt == inputs.end()) return;

                       auto lhsTypeIt = typeMap.find(lhsIt->second);
                       auto rhsTypeIt = typeMap.find(rhsIt->second);

                       if (lhsTypeIt == typeMap.end() || rhsTypeIt == typeMap.end()) {
                         emitDiag(result.diagnostics,
                                  FullID{declId, nodeId},
                                  diagnostic::Code::ERROR_CANNOT_DETERMINE_TYPE_OF_LOCAL,
                                  "");
                         return;
                       }

                       auto lhsName = symbolTable.getType(lhsTypeIt->second)->name();
                       auto rhsName = symbolTable.getType(rhsTypeIt->second)->name();
                       emitDiag(result.diagnostics,
                                FullID{declId, nodeId},
                                diagnostic::Code::ERROR_OPERATOR_OVERLOAD_RESOLUTION_FAILED,
                                fmt::format(
                                  "No binary {} exists with operand types {}, {}.", stringify(b.op), lhsName, rhsName));
                     },
                     [&](const pt::Unary& u) {
                       auto inMapIt = inputMap.find(nodeId);
                       if (inMapIt == inputMap.end()) return;

                       const auto& inputs = inMapIt->second;
                       auto operandIt = inputs.find(0);
                       if (operandIt == inputs.end()) return;

                       auto operandTypeIt = typeMap.find(operandIt->second);
                       if (operandTypeIt == typeMap.end()) {
                         emitDiag(result.diagnostics,
                                  FullID{declId, nodeId},
                                  diagnostic::Code::ERROR_CANNOT_DETERMINE_TYPE_OF_LOCAL,
                                  "");
                         return;
                       }

                       auto typeName = symbolTable.getType(operandTypeIt->second)->name();
                       emitDiag(result.diagnostics,
                                FullID{declId, nodeId},
                                diagnostic::Code::ERROR_OPERATOR_OVERLOAD_RESOLUTION_FAILED,
                                fmt::format("No unary {} exists with operand type {}.", stringify(u.op), typeName));
                     },
                     [&](const pt::Call&) {},
                     [&](const pt::Constant&) {},
                   },
                   node);
      }

      // Check conduits flowing into call argument slots (index >= 1)
      for (const auto& [_, conduit] : func.body.conduits) {
        for (const auto& output : conduit.children) {
          // Check if the target is a call node
          auto nodeIt = func.body.nodes.find(output.target);
          if (nodeIt == func.body.nodes.end()) continue;

          auto* call = std::get_if<pt::Call>(&nodeIt->second);
          if (call == nullptr) continue;
          if (output.index < 1) continue;  // index 0 is the return slot

          auto targetIt = funcByName.find(call->target);
          if (targetIt == funcByName.end()) continue;

          const auto* targetFunc = targetIt->second;
          if (!targetFunc->input) continue;

          // Find the parameter at this index (params are 0-indexed, conduit args are 1-indexed)
          int paramIndex = output.index - 1;
          const pt::FunctionDecl::Parameter* targetParam = nullptr;
          for (const auto& param : targetFunc->input->parameters) {
            if (param.index == paramIndex) {
              targetParam = &param;
              break;
            }
          }
          if (targetParam == nullptr) continue;

          auto paramTypeId = symbolTable.getTypeID(targetParam->typeName);
          if (paramTypeId == types::ID_INVALID) continue;

          auto sourceTypeIt = typeMap.find(conduit.input);
          if (sourceTypeIt == typeMap.end()) {
            // Source has no type - cannot determine
            emitDiag(result.diagnostics,
                     FullID{declId, output.target},
                     diagnostic::Code::ERROR_CANNOT_DETERMINE_TYPE_OF_LOCAL,
                     "");
            continue;
          }

          auto sourceType = sourceTypeIt->second;
          if (sourceType != paramTypeId && !symbolTable.canImplicitlyConvert(sourceType, paramTypeId)) {
            auto fromName = symbolTable.getType(sourceType)->name();
            auto toName = symbolTable.getType(paramTypeId)->name();
            emitDiag(result.diagnostics,
                     FullID{declId, conduit.input},
                     diagnostic::Code::ERROR_INCOMPATIBLE_TYPE,
                     fmt::format("Cannot implicitly convert '{}' to '{}'.", fromName, toName));
          }
        }
      }

      // Check conduits flowing into return nodes
      if (func.output && func.output->ret) {
        auto returnId = func.output->ret->id;
        auto retTypeId = symbolTable.getTypeID(func.output->ret->typeName);

        for (const auto& [_, conduit] : func.body.conduits) {
          for (const auto& output : conduit.children) {
            if (output.target != returnId) continue;

            auto sourceTypeIt = typeMap.find(conduit.input);
            if (sourceTypeIt == typeMap.end()) continue;

            if (retTypeId == types::ID_INVALID) continue;

            auto sourceType = sourceTypeIt->second;
            if (sourceType != retTypeId && !symbolTable.canImplicitlyConvert(sourceType, retTypeId)) {
              auto fromName = symbolTable.getType(sourceType)->name();
              auto toName = symbolTable.getType(retTypeId)->name();
              emitDiag(result.diagnostics,
                       FullID{declId, 0, returnId},
                       diagnostic::Code::ERROR_INCOMPATIBLE_TYPE,
                       fmt::format("Cannot implicitly convert '{}' to '{}'.", fromName, toName));
            }
          }
        }
      }
    }

    std::ranges::sort(result.diagnostics, [](const auto& a, const auto& b) {
      return std::get<FullID>(a.location) < std::get<FullID>(b.location);
    });

    return result;
  }

}  // namespace fluir::lsp
