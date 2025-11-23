#include "compiler/types/type.hpp"

namespace fluir::types {
  Type::Type(std::string name) : name_(std::move(name)) { }
}  // namespace fluir::types
