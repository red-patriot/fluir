#include "vm/decoder/decode.hpp"

#include <stdexcept>

#include "vm/decoder/inspect.hpp"
#include "vm/exception.hpp"

using namespace std::string_literals;

namespace fluir {
  code::Header decodeHeader(std::string_view source) {
    if (source.empty()) {
      throw InvalidBytecodeFile{"Bytecode file was unexpectedly empty."};
    }
    switch (source.at(0)) {
      case 'I':
        return InspectDecoder{}.decodeHeader(source);
      default:
        throw InvalidBytecodeFile{ "'"s + source.at(0) + "' is an invalid bytecode file type."s};
    }
  }

  code::ByteCode decode(std::string_view source) {
    if (source.size() >= 1) {
      switch (source.at(0)) {
        case 'I':
          return InspectDecoder{}.decode(source);
      }
    }
    throw std::runtime_error("Invalid header. File type must be 'I' (0x4C).");
  }
}  // namespace fluir
