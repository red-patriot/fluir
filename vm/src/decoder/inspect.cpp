module;

#include <charconv>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <vector>

module fluir.decoder.inspect;

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
    auto constantBlock = constants();
    auto codeBlock = code();
    // TODO: Check for errors

    code_.chunks.push_back(
      code::Chunk{.name = std::string{name.source}, .code = codeBlock, .constants = constantBlock});
  }

  std::vector<code::Value> InspectDecoder::constants() {
    [[maybe_unused]] auto constSection = scanNext();
    auto rawCount = scanNext();
    auto count = toUnsignedInteger(rawCount);

    std::vector<code::Value> constants;
    for (size_t i = 0; i != count; ++i) {
      constants.push_back(decodeConstant());
    }

    return constants;
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
    switch (start_[0]) {
      case 'C':
        if (current_ - start_ > 1) {
          switch (start_[1]) {
            case 'H':
              return checkKeyword("CHUNK", TokenType::CHUNK);
            case 'O':
              if (current_ - start_ > 2) {
                switch (start_[2]) {
                  case 'D':
                    return checkKeyword("CODE", TokenType::CODE);
                  case 'N':
                    return checkKeyword("CONSTANTS", TokenType::CONSTANTS);
                }
              }
          }
        }
        break;
      case 'I':
        return checkInstruction();
        break;
      case 'V':
        return checkValueType();
      case 'x':
        return TokenType::HEX_LITERAL;
    }
    return TokenType::IDENTIFIER;
  }
  TokenType InspectDecoder::checkKeyword(std::string_view expected, TokenType type) {
    std::string_view current{start_, current_};
    if (current.size() == expected.size() && current == expected) {
      return type;
    }
    return TokenType::IDENTIFIER;
  }

  TokenType InspectDecoder::checkValueType() {
    if (current_ - start_ > 1) {
      switch (start_[1]) {
        case 'F':
          return checkKeyword("VFP", TokenType::TYPE_FP);
      }
    }
    return TokenType::IDENTIFIER;
  }

  TokenType InspectDecoder::checkInstruction() {
    if (current_ - start_ > 1) {
      switch (start_[1]) {
        case 'E':
          return checkKeyword("IEXIT", TokenType::INST_EXIT);
        case 'F':
          return checkFPInstruction();
        case 'P':
          if (current_ - start_ > 2) {
            switch (start_[2]) {
              case 'O':
                return checkKeyword("IPOP", TokenType::INST_POP);
              case 'U':
                return checkKeyword("IPUSH_FP", TokenType::INST_PUSH_FP);
            }
          }
          break;
      }
    }
    return TokenType::IDENTIFIER;
  }

  TokenType InspectDecoder::checkFPInstruction() {
    std::string_view current{start_, current_};
    if (current.starts_with("IFP_") && current.size() > 4) {
      switch (current[4]) {
        case 'A':
          if (current.size() > 5) {
            switch (current[5]) {
              case 'D':
                return checkKeyword("IFP_ADD", TokenType::INST_FP_ADD);
              case 'F':
                return checkKeyword("IFP_AFFIRM", TokenType::INST_FP_AFFIRM);
            }
          }
          break;
        case 'D':
          return checkKeyword("IFP_DIVIDE", TokenType::INST_FP_DIVIDE);
        case 'M':
          return checkKeyword("IFP_MULTIPLY", TokenType::INST_FP_MULTIPLY);
        case 'N':
          return checkKeyword("IFP_NEGATE", TokenType::INST_FP_NEGATE);
        case 'S':
          return checkKeyword("IFP_SUBTRACT", TokenType::INST_FP_SUBTRACT);
      }
    }
    return TokenType::IDENTIFIER;
  }

  Token InspectDecoder::createToken(TokenType type) {
    return Token{.type = type, .source = std::string_view{start_, current_}};
  }

  size_t InspectDecoder::toUnsignedInteger(Token rawNumber) {
    if (rawNumber.type != TokenType::HEX_LITERAL) {
      throw std::runtime_error{"Expected a HEX literal."};
    }
    size_t number;
    // Skip the 'x' at the beginning of the source when converting
    std::from_chars(rawNumber.source.data() + 1, rawNumber.source.data() + rawNumber.source.size(), number, 16);
    // TODO: Check error
    return number;
  }
  std::uint8_t InspectDecoder::decodeInstruction() {
    auto token = scanNext();
    switch (token.type) {
      case TokenType::INST_EXIT:
        return code::Instruction::EXIT;
      case TokenType::INST_PUSH_FP:
        return code::Instruction::PUSH_FP;
      case TokenType::INST_POP:
        return code::Instruction::POP;
      case TokenType::INST_FP_ADD:
        return code::Instruction::FP_ADD;
      case TokenType::INST_FP_SUBTRACT:
        return code::Instruction::FP_SUBTRACT;
      case TokenType::INST_FP_MULTIPLY:
        return code::Instruction::FP_MULTIPLY;
      case TokenType::INST_FP_DIVIDE:
        return code::Instruction::FP_DIVIDE;
      case TokenType::INST_FP_AFFIRM:
        return code::Instruction::FP_AFFIRM;
      case TokenType::INST_FP_NEGATE:
        return code::Instruction::FP_NEGATE;
      case TokenType::HEX_LITERAL:
        // This is an operand to one of the above instructions
        return toUnsignedInteger(token);
      default:
        throw std::runtime_error{"Expected a decodable instruction"};
    }
  }
  code::Value InspectDecoder::decodeConstant() {
    auto type = scanNext();
    switch (type.type) {
      case TokenType::TYPE_FP:
        return decodeFloatConstant();
      default:
        throw std::runtime_error{"Expected a Type keyword."};
    }
  }

  code::Value InspectDecoder::decodeFloatConstant() {
    auto rawConstant = scanNext();
    double number;
    std::from_chars(rawConstant.source.data(), rawConstant.source.data() + rawConstant.source.size(), number);
    // TODO: Check error
    return number;
  }
}  // namespace fluir
