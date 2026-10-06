#include "compiler/types/type.hpp"

namespace fluir::types {
  Type::Type(std::string name) : name_(std::move(name)) { }

  Type::Type(std::string name, Category category) : name_(std::move(name)), category_(category) { }

  Product::Product(std::string name, std::vector<TypeID> elements) :
    Type(std::move(name), Category::Product), elements_(elements) { }
}  // namespace fluir::types
