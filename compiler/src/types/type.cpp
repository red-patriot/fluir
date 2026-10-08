#include "compiler/types/type.hpp"

#include <fmt/format.h>

namespace fluir::types {
  Type::Type(std::string name) : name_(std::move(name)) { }

  Type::Type(std::string name, Category category) : name_(std::move(name)), category_(category) { }

  Product::Product(std::string name, std::vector<TypeID> elements) :
    Type(std::move(name), Category::Product), elements_(std::move(elements)) { }

  std::string Product::anonymousName(const std::vector<TypeID>& elements) {
    return fmt::format("__product_{}", fmt::join(elements, "_"));
  }

  Product Product::anonymous(std::vector<TypeID> elements) {
    return Product{anonymousName(elements), std::move(elements)};
  }

}  // namespace fluir::types
