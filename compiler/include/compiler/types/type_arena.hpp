#ifndef FLUIR_COMPILER_TYPES_TYPE_ARENA_HPP
#define FLUIR_COMPILER_TYPES_TYPE_ARENA_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "compiler/types/type.hpp"

namespace fluir::types {
  class TypeArena {
   public:
    TypeArena();

    Type const* findTypeNamed(std::string_view name) const;

   private:
    std::unordered_set<std::unique_ptr<Type>> types_{};
    std::unordered_map<std::string_view, Type const*> typeNames_{};
  };
}  // namespace fluir::types

#endif
