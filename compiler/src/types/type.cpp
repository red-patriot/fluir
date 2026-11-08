#include "compiler/types/type.hpp"

#include <ranges>

#include <fmt/format.h>
#include <fmt/ranges.h>

namespace fluir::types {
  Type::Type(std::string name) : name_(std::move(name)) { }

  Type::Type(std::string name, Category category) : name_(std::move(name)), category_(category) { }

  const Scalar& F64() {
    static Scalar f64{Scalar{"F64"}};
    return f64;
  }
  const Scalar& I8() {
    static Scalar i8{Scalar{"I8"}};
    return i8;
  }
  const Scalar& I16() {
    static Scalar i16{Scalar{"I16"}};
    return i16;
  }
  const Scalar& I32() {
    static Scalar i32{Scalar{"I32"}};
    return i32;
  }
  const Scalar& I64() {
    static Scalar i64{Scalar{"I64"}};
    return i64;
  }
  const Scalar& U8() {
    static Scalar u8{Scalar{"U8"}};
    return u8;
  }
  const Scalar& U16() {
    static Scalar u16{Scalar{"U16"}};
    return u16;
  }
  const Scalar& U32() {
    static Scalar u32{Scalar{"U32"}};
    return u32;
  }
  const Scalar& U64() {
    static Scalar u64{Scalar{"U64"}};
    return u64;
  }
  const Scalar& BOOL() {
    static Scalar bool_{Scalar{"BOOL"}};
    return bool_;
  }
  const Scalar& MAGIC_ANY_TYPE() {
    static Scalar magicAnyType{Scalar{"__MAGIC_ANY_TYPE__"}};
    return magicAnyType;
  }
  const Product& EMPTY() {
    static Product empty{Product{"()", {}}};
    return empty;
  }

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
