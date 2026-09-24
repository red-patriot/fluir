#ifndef FLUIR_EDITOR_CORE_INTELLIGENCE_HPP
#define FLUIR_EDITOR_CORE_INTELLIGENCE_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/operator.hpp"
#include "editor/core/field.hpp"

namespace fluir::editor {
  namespace intelligence {
    struct ParamInfo {
      std::string name;
      std::string typeName;
    };

    struct FunctionDecl {
      std::string name;
      std::vector<ParamInfo> parameters;
      std::optional<std::string> returnTypeName;
    };

    struct ModuleData {
      std::unordered_map<ID, FunctionDecl> functions;
    };
  }  // namespace intelligence

  struct FunctionDefOption { };
  struct CommentOption { };
  struct OperatorOption {
    Operator op;
    enum Arity { UNARY, BINARY } arity;  // TODO: Generalize this?
  };
  struct ConstantOption {
    literals_types::Literal value;
  };
  struct CallFunctionOption {
    std::string_view target;
    std::vector<intelligence::ParamInfo> parameters;
    std::optional<std::string_view> returnType;
  };
  struct ConditionalOption { };

  using CompletionOption = std::
    variant<FunctionDefOption, CommentOption, OperatorOption, ConstantOption, CallFunctionOption, ConditionalOption>;

  struct Completion {
    std::string label;
    CompletionOption option;
  };

  /** Answers what may go where in a tree. Type- and module-aware answers slot in behind the same calls. */
  class Intelligence {
   public:
    /** Loads a module into memory, parsing its information for intelligence.
     * \return true if the module was loaded successfully, false on error
     */
    bool load(std::optional<std::filesystem::path> programPath, const pt::ParseTree& tree);

    /** Erases a loaded module from memory. No op if the module has not been loaded.
     * \return true if the module was erased, false on error (or not previously loaded)
     */
    bool unload(std::optional<std::filesystem::path> programPath);

    /** What `field` at `path` may be picked from; empty for a typed field or a missing one. */
    std::vector<std::string> choices(const pt::ParseTree& tree, const FullID& path, Field field) const;

    /** Completions available inside `body`. Empty `body` indicates a top-level addition, otherwise completions will be
     * applied inside `body`. */
    std::vector<Completion> completions(const pt::ParseTree& tree, const FullID& body) const;

   private:
    // TODO: Allow for multiple modules to be open
    intelligence::ModuleData module_;

    /** Returns a vector of builtin operations available in a decl body (operators, functions, etc). */
    static const std::vector<Completion>& bodyBuiltins();
    /** Returns completions available at the given body location (functions to call, etc). */
    std::vector<Completion> completionsAt(const pt::Block& body) const;
  };

}  // namespace fluir::editor

#endif
