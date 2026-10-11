#ifndef FLUIR_COMPILER_BACKEND_BYTECODE_GENERATOR_HPP
#define FLUIR_COMPILER_BACKEND_BYTECODE_GENERATOR_HPP

#include <stack>

#include "compiler/backend/code_writer.hpp"
#include "compiler/models/ast.hpp"
#include "compiler/utility/context.hpp"

namespace fluir {
  void generateCode(Context& ctx, const ast::AST& graph, CodeWriter& writer);

  class BytecodeGenerator {
   public:
    static void generate(Context& ctx, const ast::AST& graph, CodeWriter& writer);

    void operator()(const ast::FunctionDecl& func);

    int generate(const ast::BinaryOp& binary);
    int generate(const ast::UnaryOp& unary);
    int generate(const ast::Constant& constant);
    int generate(const ast::Cast& cast);
    int generate(const ast::LocalWrite& write);
    int generate(const ast::LocalRead& read);
    int generate(const ast::Call& call);
    int generate(const ast::Conditional& conditional);

   private:
    Context& ctx_;
    const ast::AST& graph_;
    CodeWriter& writer_;

    code::Header header_;
    be::ConstantsArray constants_;
    std::vector<code::Chunk> chunks_;
    code::Chunk* current_{nullptr};
    std::unordered_map<std::string_view, size_t> functionIndices_;

    struct Scope {
      std::unordered_map<ID, size_t> slots{};
      size_t outputCount{0};
    };
    std::stack<Scope> scopes_;

    BytecodeGenerator(Context& ctx, CodeWriter& writer, const ast::AST& graph);

    void emitByte(std::uint8_t byte);
    void emitBytes(std::uint8_t byte1, std::uint8_t byte2);
    void emitLongOperand(std::uint64_t arg);
    /** Emits a jump instruction with a placeholder operand to be patched later.
     * \return the index to write the patch to. */
    size_t emitJump(code::Instruction instruction);
    /** Patches a previously emitted jump.
     * `jumpIndex` is the index where the patch should be written to, `targetIndex` is where the ip should land when the
     * jump is executed. */
    void patchJump(size_t jumpIndex, size_t targetIndex);
    size_t addConstant(be::Constant value);

    void run();
    /*Generate bytecode for a subgraph, returns the number of stack slots added or removed by this code */
    int generate(const ast::DataFlowGraph& subgraph, const std::vector<ID>& inputs, const std::vector<ID>& outputs);
    /*Generate bytecode for a dependency, returns the number of stack slots added or removed by this code */
    int recursivelyGenerate(const ast::Dependency& dependency);
    /*Generate bytecode for a node, returns the number of stack slots added or removed by this code */
    int recursivelyGenerate(const ast::Node& node);

    /** Starts a new scope, with its own local variables and outputs */
    Scope& pushScope();
    /** Cleans up the top scope on the stack, including popping all its local variables and temporaries. */
    void popScope(int stackTemporaries);

    /** Determines if a POP instruction should be emitted after
     * executing this node as a top-level expression */
    bool shouldPopAfter(const ast::Node& node);
    /** Indicates if the given node type leaves values on the stack to be popped */
    bool pushesToStack(const ast::Node& node);

    void emitFloatOperator(const Operator op, bool unary = false);
    void emitIntOperator(const Operator op, bool unary = false);
    void emitUintOperator(const Operator op, bool unary = false);
    void emitBoolOperator(const Operator op, bool unary = false);
    void emitWidthCast(types::TypeID sourceType, types::TypeID targetType);
  };
}  // namespace fluir

#endif
