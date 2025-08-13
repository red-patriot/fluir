#ifndef FLUIR_VM_DECODER_INSPECT_HPP
#define FLUIR_VM_DECODER_INSPECT_HPP

#include <string>

#include "bytecode/byte_code.hpp"

namespace fluir {
  // clang-format off
  enum class TokenType {
    // Literals
    HEX_LITERAL, FLOAT_LITERAL, IDENTIFIER,
    // Sections
    CHUNK, CODE, CONSTANTS,
    // Data Types
    TYPE_FP,
    // Instructions
    INST_EXIT,
    INST_PUSH_FP, INST_POP,
    INST_FP_ADD, INST_FP_SUBTRACT, INST_FP_MULTIPLY, INST_FP_DIVIDE,
    INST_FP_AFFIRM, INST_FP_NEGATE,
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
    code::Header decodeHeader(const std::string_view source);
    code::ByteCode decode(code::Header header, const std::string_view source);

   private:
    std::string_view source_;
    char const* start_{nullptr};
    char const* current_{nullptr};
    size_t line_{0};
    code::ByteCode code_;

    static constexpr size_t HEADER_LEN = 23; // The number of characters in the header

    void reset(std::string_view source);

    void decodeHeader();
    void decodeChunks();

    void chunk();
    std::vector<code::Value> constants();
    std::vector<uint8_t> code();
    Token identifier();
    Token number();

    bool atEnd();
    char next();
    char peek();
    Token scanNext();
    void eatWhitespace();

    TokenType decodeIdentifierType();
    TokenType checkKeyword(std::string_view expected, TokenType type);
    TokenType checkValueType();
    TokenType checkInstruction();
    TokenType checkFPInstruction();
    Token createToken(TokenType type);
    size_t toUnsignedInteger(Token rawNumber);

    std::uint8_t decodeInstruction();
    code::Value decodeConstant();
    code::Value decodeFloatConstant();
  };
}  // namespace fluir

#endif
