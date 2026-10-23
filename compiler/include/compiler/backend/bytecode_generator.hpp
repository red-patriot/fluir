#ifndef FLUIR_COMPILER_BACKEND_BYTECODE_GENERATOR_HPP
#define FLUIR_COMPILER_BACKEND_BYTECODE_GENERATOR_HPP

#include <stack>

#include "compiler/backend/code_writer.hpp"
#include "compiler/models/ast.hpp"
#include "compiler/utility/context.hpp"
#include "func_frame.hpp"

namespace fluir {
  void generateCode(Context& ctx, const ast::AST& graph, CodeWriter& writer);

  class BytecodeGenerator {
   public:
    static void generate(Context& ctx, const ast::AST& graph, CodeWriter& writer);

    void operator()(const ast::FunctionDecl& func);

    void generate(const ast::BinaryOp& binary);
    void generate(const ast::UnaryOp& unary);
    void generate(const ast::Constant& constant);
    void generate(const ast::Cast& cast);
    void generate(const ast::LocalWrite& write);
    void generate(const ast::LocalRead& read);
    void generate(const ast::Call& call);
    void generate(const ast::Conditional& conditional);

   private:
    Context& ctx_;
    const ast::AST& graph_;
    CodeWriter& writer_;

    code::Header header_;
    be::ConstantsArray constants_;
    std::vector<code::Chunk> chunks_;
    code::Chunk* current_{nullptr};
    std::unordered_map<std::string_view, size_t> functionIndices_;
    be::BytecodeFrame* frame_{nullptr};
    be::Scope* currentScope_{nullptr};

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
    /** Generates bytecode for a data flow graph*/
    void generate(const ast::DataFlowGraph& dfg,
                  const std::vector<WithID<types::TypeID>>& inputs,
                  const std::vector<WithID<types::TypeID>>& outputs);
    /*Generate bytecode for a subgraph */
    void generate(const ast::SubGraph& subgraph,
                  const std::vector<WithID<types::TypeID>>& inputs,
                  const std::vector<WithID<types::TypeID>>& outputs);
    /*Generate bytecode for a dependency and leaves the result on the stack */
    void generateDependency(const ast::Dependency& dependency);
    /*Generate bytecode for a top-level node, and pops the final result off the stack when done */
    void generateTopLevel(const ast::Node& node);
    /** Generates bytecode for a node, leaving it on the stack */
    void doGenerate(const ast::Node& node);

    /** Adds a local to the current scope, mapped by ID. */
    uint8_t addLocalSlot(const ID& id, types::TypeID type);

    void emitFloatOperator(const Operator op, bool unary = false);
    void emitIntOperator(const Operator op, bool unary = false);
    void emitUintOperator(const Operator op, bool unary = false);
    void emitBoolOperator(const Operator op, bool unary = false);
    void emitWidthCast(types::TypeID sourceType, types::TypeID targetType);
  };
}  // namespace fluir

#endif
