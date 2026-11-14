#include "vm/decoder/inspect.hpp"

#include <charconv>
#include <format>
#include <stdexcept>
#include <string>

#include "fluir/util/macros.hpp"
#include "fluir/util/trie.hpp"

namespace fluir {
  code::ByteCode InspectDecoder::decode(const std::string_view source) {
    source_ = source;
    line_ = 1;
    code_ = code::ByteCode{};

    decodeHeader();
    start_ = source_.data();
    current_ = source_.data();

    decodeChunks();

    return std::move(code_);
  }

  void InspectDecoder::decodeHeader() {
    code::Header header;

    header.filetype = source_.at(0);

    auto majorStr = source_.substr(1, 2);
    auto result = std::from_chars(majorStr.data(), majorStr.data() + majorStr.size(), header.major, 16);

    auto minorStr = std::string_view(result.ptr, 2);
    result = std::from_chars(minorStr.data(), minorStr.data() + minorStr.size(), header.minor, 16);

    auto patchStr = std::string_view(result.ptr, 2);
    result = std::from_chars(patchStr.data(), patchStr.data() + patchStr.size(), header.patch, 16);

    auto entryOffsetStr = std::string_view(result.ptr, 16);
    result =
      std::from_chars(entryOffsetStr.data(), entryOffsetStr.data() + entryOffsetStr.size(), header.entryOffset, 16);

    // Consume the header part of the string
    source_ = std::string_view{result.ptr, source_.data() + source_.size()};

    code_.header = header;
  }

  void InspectDecoder::decodeChunks() {
    while (!atEnd()) {
      Token nextToken = scanNext();
      switch (nextToken.type) {
        case TokenType::CONSTANTS:
          constants();
          break;
        case TokenType::CHUNK:
          chunk();
          break;
        case TokenType::END_OF_FILE:
          break;
        default:
          throw std::runtime_error("Invalid bytecode. Expected token 'CHUNK'.");
      }
    }
  }

  void InspectDecoder::chunk() {
    auto name = scanNext();
    auto codeBlock = code();
    auto inCountVal = inCount();
    auto outCountVal = outCount();
    // TODO: Check for errors

    code_.chunks.push_back(
      code::Chunk{.name = std::string{name.source}, .code = codeBlock, .inCount = inCountVal, .outCount = outCountVal});
  }

  void InspectDecoder::constants() {
    if (!code_.constants.empty()) {
      throw std::runtime_error{"Expected only one CONSTANTS section in the code."};
    }
    auto rawCount = scanNext();
    auto count = toUnsignedInteger(rawCount);

    std::vector<code::Value> constants;
    for (size_t i = 0; i != count; ++i) {
      constants.push_back(decodeConstant());
    }

    code_.constants = std::move(constants);
  }

  std::vector<uint8_t> InspectDecoder::code() {
    [[maybe_unused]] auto codeSection = scanNext();
    auto rawCount = scanNext();
    auto count = toUnsignedInteger(rawCount);

    std::vector<uint8_t> code;
    for (size_t i = 0; i != count; ++i) {
      code.push_back(decodeInstruction());
    }

    return code;
  }

  std::uint8_t InspectDecoder::inCount() {
    [[maybe_unused]] auto inSection = scanNext();
    auto rawCount = scanNext();
    auto count = toUnsignedInteger(rawCount);
    return static_cast<std::uint8_t>(count);
  }

  std::uint8_t InspectDecoder::outCount() {
    [[maybe_unused]] auto outSection = scanNext();
    auto rawCount = scanNext();
    auto count = toUnsignedInteger(rawCount);
    return static_cast<std::uint8_t>(count);
  }

  Token InspectDecoder::identifier() {
    while (std::isalnum(peek()) || peek() == '_') {
      next();
    }
    return createToken(decodeIdentifierType());
  }

  Token InspectDecoder::number() {
    while (std::isdigit(peek())) {
      next();
    }

    if (peek() == '.') {
      next();
    }
    while (std::isdigit(peek())) {
      next();
    }

    return createToken(TokenType::FLOAT_LITERAL);
  }

  bool InspectDecoder::atEnd() { return current_ == source_.data() + source_.size(); }

  char InspectDecoder::next() {
    current_++;
    return current_[-1];
  }

  char InspectDecoder::peek() { return *current_; }

  Token InspectDecoder::scanNext() {
    eatWhitespace();
    start_ = current_;
    if (atEnd()) {
      return Token{.type = TokenType::END_OF_FILE, .source{}};
    }
    char c = next();
    if (std::isalpha(c)) {
      return identifier();
    }
    if (std::isdigit(c)) {
      return number();
    }

    return Token{.type = TokenType::ERR, .source{}};
  }

  void InspectDecoder::eatWhitespace() {
    for (;;) {
      switch (peek()) {
        case '\n':
          ++line_;
          [[fallthrough]];
        case ' ':
        case '\t':
        case '\r':
          next();
          break;
        default:
          return;
      }
    }
  }

  TokenType InspectDecoder::decodeIdentifierType() {
    static const util::Trie keywords{TokenType::IDENTIFIER,
                                     {{"CHUNK", TokenType::CHUNK},
                                      {"CODE", TokenType::CODE},
                                      {"CONSTANTS", TokenType::CONSTANTS},
                                      {"IN", TokenType::IN},
                                      {"OUT", TokenType::OUT},
#define FLUIR_INSTRUCTION_BRANCHES(code) {FLUIR_STRINGIFY(FLUIR_CCAT(I, code)), TokenType::FLUIR_CCAT(INST_, code)},
                                      FLUIR_CODE_INSTRUCTIONS(FLUIR_INSTRUCTION_BRANCHES)
#undef FLUIR_INSTRUCTION_BRANCHES
                                        {"VF64", TokenType::TYPE_F64},
                                      {"VI8", TokenType::TYPE_I8},
                                      {"VI16", TokenType::TYPE_I16},
                                      {"VI32", TokenType::TYPE_I32},
                                      {"VI64", TokenType::TYPE_I64},
                                      {"VU8", TokenType::TYPE_U8},
                                      {"VU16", TokenType::TYPE_U16},
                                      {"VU32", TokenType::TYPE_U32},
                                      {"VU64", TokenType::TYPE_U64},
                                      {"VSTR", TokenType::TYPE_STR},
                                      {"VTRUE", TokenType::TYPE_TRUE},
                                      {"VFALSE", TokenType::TYPE_FALSE}}};

    std::string_view word{start_, current_};
    TokenType tokenType;
    if (word.starts_with('x')) {
      tokenType = TokenType::HEX_LITERAL;
    } else if (word.starts_with('s')) {
      tokenType = TokenType::STR_LITERAL;
    } else {
      tokenType = keywords.at(word);
    }

    return tokenType;
  }

  Token InspectDecoder::createToken(TokenType type) {
    return Token{.type = type, .source = std::string_view{start_, current_}};
  }

  size_t InspectDecoder::toUnsignedInteger(Token rawNumber) {
    if (rawNumber.type != TokenType::HEX_LITERAL) {
      throw std::runtime_error{"Expected a HEX literal."};
    }
    size_t number = 0;
    // Skip the 'x' at the beginning of the source when converting
    std::from_chars(rawNumber.source.data() + 1, rawNumber.source.data() + rawNumber.source.size(), number, 16);
    // TODO: Check error
    return number;
  }

  std::uint8_t InspectDecoder::decodeInstruction() {
    auto token = scanNext();
    switch (token.type) {
#define FLUIR_TRANSLATE_INSTRUCTION(Inst) \
  case TokenType::INST_##Inst:            \
    return code::Instruction::Inst;

      FLUIR_CODE_INSTRUCTIONS(FLUIR_TRANSLATE_INSTRUCTION)
#undef FLUIR_TRANSLATE_INSTRUCTION

      case TokenType::HEX_LITERAL:
        // This is an operand to one of the above instructions
        // TODO: Handle sizes that are too large
        return static_cast<std::uint8_t>(toUnsignedInteger(token));
      default:
        throw std::runtime_error{"Expected a decodable instruction"};
    }
  }

  code::Value InspectDecoder::decodeConstant() {
    auto type = scanNext();
    switch (type.type) {
      case TokenType::TYPE_F64:
        return decodeFloatConstant();
      case TokenType::TYPE_I8:
        return decodeIntConstant(code::PrimitiveType::I8);
      case TokenType::TYPE_I16:
        return decodeIntConstant(code::PrimitiveType::I16);
      case TokenType::TYPE_I32:
        return decodeIntConstant(code::PrimitiveType::I32);
      case TokenType::TYPE_I64:
        return decodeIntConstant(code::PrimitiveType::I64);
      case TokenType::TYPE_U8:
        return decodeIntConstant(code::PrimitiveType::U8);
      case TokenType::TYPE_U16:
        return decodeIntConstant(code::PrimitiveType::U16);
      case TokenType::TYPE_U32:
        return decodeIntConstant(code::PrimitiveType::U32);
      case TokenType::TYPE_U64:
        return decodeIntConstant(code::PrimitiveType::U64);
      case TokenType::TYPE_STR:
        return decodeStrConstant();
      case TokenType::TYPE_TRUE:
        return code::Value{true};
      case TokenType::TYPE_FALSE:
        return code::Value{false};
      default:
        throw std::runtime_error{"Expected a type keyword."};
    }
  }

  code::Value InspectDecoder::decodeFloatConstant() {
    auto rawConstant = scanNext();
    if (rawConstant.type != TokenType::FLOAT_LITERAL) {
      throw std::runtime_error{"Expected a float constant."};
    }
    double number;
    std::from_chars(rawConstant.source.data(), rawConstant.source.data() + rawConstant.source.size(), number);
    // TODO: Check error
    return code::Value{number};
  }

  code::Value InspectDecoder::decodeIntConstant(code::PrimitiveType type) {
    auto rawConstant = scanNext();
    if (rawConstant.type != TokenType::HEX_LITERAL) {
      throw std::runtime_error{"Expected a HEX literal."};
    }
    auto number = toUnsignedInteger(rawConstant);
    switch (type) {
      case code::PrimitiveType::I8:
        return code::Value{static_cast<std::int8_t>(number)};
      case code::PrimitiveType::I16:
        return code::Value{static_cast<std::int16_t>(number)};
      case code::PrimitiveType::I32:
        return code::Value{static_cast<std::int32_t>(number)};
      case code::PrimitiveType::I64:
        return code::Value{static_cast<std::int64_t>(number)};
      case code::PrimitiveType::U8:
        return code::Value{static_cast<std::uint8_t>(number)};
      case code::PrimitiveType::U16:
        return code::Value{static_cast<std::uint16_t>(number)};
      case code::PrimitiveType::U32:
        return code::Value{static_cast<std::uint32_t>(number)};
      case code::PrimitiveType::U64:
        return code::Value{static_cast<std::uint64_t>(number)};
      default:
        throw std::runtime_error{"Expected an integer type"};
    }
  }

  code::Value InspectDecoder::decodeStrConstant() {
    auto rawConstant = scanNext();
    if (rawConstant.type != TokenType::STR_LITERAL) {
      throw std::runtime_error{"Expected a string constant."};
    }
    if (rawConstant.source.size() < 9) {
      throw std::runtime_error{
        std::format("String literal '{}' is not well formed. Expected 8 characters in the size.", rawConstant.source)};
    }

    Token rawSize{.type = TokenType::HEX_LITERAL, .source = rawConstant.source.substr(0, 9)};
    auto size = toUnsignedInteger(rawSize);
    auto literalSize =
      rawConstant.source.size() - rawSize.source.size();  // Get the size of the string bit of the literal
    if (literalSize > size) {
      throw std::runtime_error{std::format("String literal '{}' is too long.", rawConstant.source)};
    }
    if (literalSize == size) {
      return code::Value{createStaticString(rawConstant.source.substr(rawSize.source.size()))};
    }
    throw std::runtime_error{"TODO: Handle string literals with spaces in them"};
  }
}  // namespace fluir
