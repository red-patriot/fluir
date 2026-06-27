#ifndef FLUIR_VM_DECODER_INSPECT_HPP
#define FLUIR_VM_DECODER_INSPECT_HPP

#include <string>

#include "vm/code/byte_code.hpp"

namespace fluir {
  // clang-format off
  enum class TokenType {
    // Literals
    HEX_LITERAL, FLOAT_LITERAL, STR_LITERAL, IDENTIFIER,
    // Sections
    CHUNK, CODE, CONSTANTS, IN, OUT,
    // Data Types
    TYPE_F64,
    TYPE_I8, TYPE_I16, TYPE_I32, TYPE_I64,
    TYPE_U8, TYPE_U16, TYPE_U32, TYPE_U64,
    TYPE_STR,
    TYPE_TRUE, TYPE_FALSE,
    // Instructions
#define FLUIR_INSTRUCTION_TOKEN(code) INST_## code,
    FLUIR_CODE_INSTRUCTIONS(FLUIR_INSTRUCTION_TOKEN)
#undef FLUIR_INSTRUCTION_TOKEN
    // Meta Info
    END_OF_FILE, ERR
  };
  // clang-format on

  struct Token {
    TokenType type;
    std::string_view source;
  };

  class InspectDecoder {
   public:
    code::ByteCode decode(const std::string_view source);

   private:
    std::string_view source_;
    char const* start_;
    char const* current_;
    size_t line_;
    code::ByteCode code_;

    void decodeHeader();
    void decodeChunks();

    void constants();
    void chunk();
    std::vector<uint8_t> code();
    std::uint8_t inCount();
    std::uint8_t outCount();
    Token identifier();
    Token number();

    bool atEnd();
    char next();
    char peek();
    Token scanNext();
    void eatWhitespace();

    TokenType decodeIdentifierType();
    Token createToken(TokenType type);
    size_t toUnsignedInteger(Token rawNumber);

    std::uint8_t decodeInstruction();
    code::Value decodeConstant();
    code::Value decodeFloatConstant();
    code::Value decodeIntConstant(code::PrimitiveType type);
    code::Value decodeStrConstant();
  };
}  // namespace fluir

#endif
