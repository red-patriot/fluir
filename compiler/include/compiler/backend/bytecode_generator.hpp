#ifndef FLUIR_COMPILER_BACKEND_BYTECODE_GENERATOR_HPP
#define FLUIR_COMPILER_BACKEND_BYTECODE_GENERATOR_HPP

#include <stack>

#include "bytecode/byte_code.hpp"
#include "compiler/backend/code_writer.hpp"
#include "compiler/models/ast.hpp"
#include "compiler/utility/context.hpp"

namespace fluir {
  void generateCode(Context& ctx, const ast::AST& graph, CodeWriter& writer);
  void writeCode(const code::ByteCode& code, CodeWriter& writer, std::ostream& destination);

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
      size_t returnCount{0};
    };
    std::stack<Scope> scopes_;

    BytecodeGenerator(Context& ctx, CodeWriter& writer, const ast::AST& graph);

    void emitByte(std::uint8_t byte);
    void emitBytes(std::uint8_t byte1, std::uint8_t byte2);
    void emitLongOperand(std::uint64_t arg);
    size_t addConstant(be::Constant value);

    void run();
    void recursivelyGenerate(const ast::Node& node);

    Scope& pushScope();
    void popScope();

    /** Determines if a POP instruction should be emitted after
     * executing this node as a top-level expression */
    bool shouldPopAfter(const ast::Node& node);

    void emitFloatOperator(const Operator op, bool unary = false);
    void emitIntOperator(const Operator op, bool unary = false);
    void emitUintOperator(const Operator op, bool unary = false);
    void emitWidthCast(types::TypeID sourceType, types::TypeID targetType);
  };
}  // namespace fluir

#endif
