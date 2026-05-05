#include "compiler/backend/code_writer.hpp"

#include <iostream>

namespace fluir {
  CodeWriter::CodeWriter(std::ostream& os) : os_(os) { }
}  // namespace fluir
