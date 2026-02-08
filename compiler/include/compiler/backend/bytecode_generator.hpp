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

   private:
    Context& ctx_;
    const ast::AST& graph_;
    code::ByteCode code_;
    code::Chunk current_;

    struct Scope {
      std::unordered_map<ID, size_t> slots;
    };
    std::stack<Scope> scopes_;

    explicit BytecodeGenerator(Context& ctx, const ast::AST& graph);

    void emitByte(std::uint8_t byte);
    void emitBytes(std::uint8_t byte1, std::uint8_t byte2);
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

/**
 * EXPECT
 *     Which is: { '\x1', '\0', '\v', '\x1', '\x1', '\b', '\x1', '\x2', '\x3', '\0', '\x5' (5), '\x2' (2), '\x3' (3),
 * '\0', '\v' (11, 0xB), '\x2' (2), '\x2' (2), '\0' } ACTUAL Which is: { '\x1', '\0', '\v', '\x1', '\x1', '\b', '\x2',
 * '\x1', '\x2', '\x3' (3), '\0', '\x5' (5), '\x2' (2), '\x3' (3), '\0', '\v' (11, 0xB), '\x2' (2), '\0' }
 *
 *
 */
