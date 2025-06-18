module;

#include <string>

export module fluir.decoder;
export import fluir.byte_code;
import fluir.byte_code;

namespace fluir {
  export code::ByteCode decode(std::string_view source);
}  // namespace fluir
