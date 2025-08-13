#include "vm/decoder/decode.hpp"

#include <stdexcept>

#include "vm/decoder/inspect.hpp"
#include "vm/exception.hpp"

using namespace std::string_literals;

namespace fluir {
  code::ByteCode decode(std::string_view source) {
    auto header = decodeHeader(source);
    return decode(header, source);
  }

  code::Header decodeHeader(std::string_view source) {
    if (source.empty()) {
      throw InvalidBytecodeFile{"Bytecode file was unexpectedly empty."};
    }
    switch (source.at(0)) {
      case 'I':
        return InspectDecoder{}.decodeHeader(source);
      default:
        throw InvalidBytecodeFile{"'"s + source.at(0) + "' is an invalid bytecode file type."s};
    }
  }

  code::ByteCode decode(code::Header header, std::string_view source) {
    switch (header.filetype) {
      case 'I':
        return InspectDecoder{}.decode(std::move(header), source);
      default:
        throw InvalidBytecodeFile("Invalid header. File type must be 'I' (0x4C).");
    }
  }
}  // namespace fluir
