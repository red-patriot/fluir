#include "compiler/types/type.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>

namespace fluir::types {
  Type::Type(std::string name) : name_(std::move(name)) { }

  Type::Type(std::string name, Category category) : name_(std::move(name)), category_(category) { }

  Product::Product(std::string name, std::vector<TypeID> elements) :
    Type(std::move(name), Category::Product), elements_(std::move(elements)) { }

  std::string Product::nameFromTypes(const std::vector<TypeID>& elements) {
    return fmt::format("({})", fmt::join(elements, ","));
  }

  Product Product::createFromTypes(std::vector<TypeID> elements) {
    auto name = nameFromTypes(elements);
    return Product{std::move(name), std::move(elements)};
  }

}  // namespace fluir::types
