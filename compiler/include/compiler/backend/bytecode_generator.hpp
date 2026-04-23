#ifndef FLUIR_COMPILER_BACKEND_BYTECODE_GENERATOR_HPP
#define FLUIR_COMPILER_BACKEND_BYTECODE_GENERATOR_HPP

#include <stack>

#include "bytecode/byte_code.hpp"
#include "compiler/backend/code_writer.hpp"
#include "compiler/models/ast.hpp"
#include "compiler/utility/context.hpp"

namespace fluir {
  Results<code::ByteCode> generateCode(Context& ctx, const ast::AST& graph);
  void writeCode(const code::ByteCode& code, CodeWriter& writer, std::ostream& destination);

  class BytecodeGenerator {
   public:
    static Results<code::ByteCode> generate(Context& ctx, const ast::AST& graph);

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
    code::ByteCode code_;
    code::Chunk current_;
    std::unordered_map<std::string_view, size_t> functionIndices_;

    struct Scope {
      std::unordered_map<ID, size_t> slots{};
      size_t returnCount{0};
    };
    std::stack<Scope> scopes_;

    explicit BytecodeGenerator(Context& ctx, const ast::AST& graph);

    void emitByte(std::uint8_t byte);
    void emitBytes(std::uint8_t byte1, std::uint8_t byte2);
    void emitLongOperand(std::uint64_t arg);
    size_t addConstant(code::Value value);

    Results<code::ByteCode> run();
    void recursivelyGenerate(const ast::Node& node);

    Scope& pushScope();
    void popScope();

    void emitFloatOperator(const Operator op, bool unary = false);
    void emitIntOperator(const Operator op, bool unary = false);
    void emitUintOperator(const Operator op, bool unary = false);
    void emitWidthCast(types::TypeID sourceType, types::TypeID targetType);
  };
}  // namespace fluir

#endif
