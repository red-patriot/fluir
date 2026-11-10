#include "compiler/types/type_arena.hpp"

#include <algorithm>

namespace fluir::types {
  TypeArena::TypeArena() {
    std::unordered_set<TypePtr> builtinTypes{
      &EMPTY(), &F64(), &I8(), &I16(), &I32(), &I64(), &U8(), &U16(), &U32(), &U64(), &BOOL(), &MAGIC_ANY_TYPE()};
    for (const auto* t : builtinTypes) {
      typeNames_.emplace(t->name(), t);
    }
  }

  Product const* TypeArena::addProduct(const std::vector<TypePtr>& elements) {
    if (elements.empty()) {
      return &EMPTY();
    }
    if (const auto i = products_.find(std::span{elements}); i != products_.end()) {
      return i->get();
    }
    const auto i = products_.insert(std::unique_ptr<Product>{new Product(elements)});
    typeNames_.emplace((*i.first)->name(), i.first->get());
    return i.first->get();
  }

  TypePtr TypeArena::findTypeNamed(std::string_view name) const {
    if (typeNames_.contains(name)) {
      return typeNames_.at(name);
    }
    return nullptr;
  }

  size_t TypeArena::ElementwiseHash::operator()(std::span<TypePtr const> elements) const {
    size_t hash = elements.size();
    for (auto const& element : elements) {
      hash ^= std::hash<TypePtr>{}(element);
    }
    return hash;
  }

  bool TypeArena::ElementwiseEq::operator()(std::span<TypePtr const> lhs, std::span<TypePtr const> rhs) const {
    return std::ranges::equal(lhs, rhs);
  }

}  // namespace fluir::types
