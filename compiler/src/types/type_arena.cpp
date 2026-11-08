#include "compiler/types/type_arena.hpp"

namespace fluir::types {
  TypeArena::TypeArena() {
    std::unordered_set<Type const*> builtinTypes{
      &EMPTY(), &F64(), &I8(), &I16(), &I32(), &I64(), &U8(), &U16(), &U32(), &U64(), &BOOL(), &MAGIC_ANY_TYPE()};
    for (const auto* t : builtinTypes) {
      typeNames_.emplace(t->name(), t);
    }
  }

  Product const* TypeArena::addProduct(const std::vector<Type const*>& elements) {
    if (elements.empty()) {
      return &EMPTY();
    }
    if (const auto i = products_.find(std::span{elements}); i != products_.end()) {
      return i->get();
    }
    const auto i = products_.insert(std::unique_ptr<Product>{new Product(elements)});
    return i.first->get();
  }

  Type const* TypeArena::findTypeNamed(std::string_view name) const {
    if (typeNames_.contains(name)) {
      return typeNames_.at(name);
    }
    return nullptr;
  }

  size_t TypeArena::ElementwiseHash::operator()(std::span<Type const* const> elements) const {
    size_t hash = elements.size();
    for (auto const& element : elements) {
      hash ^= std::hash<Type const*>{}(element);
    }
    return hash;
  }

  bool TypeArena::ElementwiseEq::operator()(std::span<Type const* const> lhs, std::span<Type const* const> rhs) const {
    return std::ranges::equal(lhs, rhs);
  }

}  // namespace fluir::types
