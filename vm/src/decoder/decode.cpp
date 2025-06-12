module;
#include <stdexcept>
#include "bytecode/byte_code.hpp"

module fluir.decoder;
import fluir.decoder.inspect;

namespace fluir {
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
