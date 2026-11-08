#include "compiler/types/type_arena.hpp"

namespace fluir::types {
  TypeArena::TypeArena() {
    std::unordered_set<Type const*> builtinTypes{
      &EMPTY(), &F64(), &I8(), &I16(), &I32(), &I64(), &U8(), &U16(), &U32(), &U64(), &BOOL(), &MAGIC_ANY_TYPE()};
    for (const auto* t : builtinTypes) {
      typeNames_.emplace(t->name(), t);
    }
  }

  Type const* TypeArena::findTypeNamed(std::string_view name) const {
    if (typeNames_.contains(name)) {
      return typeNames_.at(name);
    }
    return nullptr;
  }
}  // namespace fluir::types
