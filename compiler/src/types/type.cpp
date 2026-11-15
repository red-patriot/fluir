#include "compiler/types/type.hpp"

#include <ranges>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "bytecode/primitives.hpp"

namespace fluir::types {

  Type::Type(std::string name, Category category) : name_(std::move(name)), category_(category) { }

  bool Scalar::isIntegral() const { return classification_ == UNSIGNED || classification_ == SIGNED; }
  bool Scalar::isFloatingPoint() const { return classification_ == FLOAT; }
  bool Scalar::isSigned() const { return classification_ == SIGNED || classification_ == FLOAT; }
  bool Scalar::isUnsigned() const { return classification_ == UNSIGNED; }
  bool Scalar::isBool() const { return classification_ == BOOL; }

  const Scalar& F64() {
    static Scalar f64{Scalar{"F64", Scalar::FLOAT, 64}};
    return f64;
  }
  const Scalar& I8() {
    static Scalar i8{Scalar{"I8", Scalar::SIGNED, 1}};
    return i8;
  }
  const Scalar& I16() {
    static Scalar i16{Scalar{"I16", Scalar::SIGNED, 2}};
    return i16;
  }
  const Scalar& I32() {
    static Scalar i32{Scalar{"I32", Scalar::SIGNED, 4}};
    return i32;
  }
  const Scalar& I64() {
    static Scalar i64{Scalar{"I64", Scalar::SIGNED, 8}};
    return i64;
  }
  const Scalar& U8() {
    static Scalar u8{Scalar{"U8", Scalar::UNSIGNED, 1}};
    return u8;
  }
  const Scalar& U16() {
    static Scalar u16{Scalar{"U16", Scalar::UNSIGNED, 2}};
    return u16;
  }
  const Scalar& U32() {
    static Scalar u32{Scalar{"U32", Scalar::UNSIGNED, 4}};
    return u32;
  }
  const Scalar& U64() {
    static Scalar u64{Scalar{"U64", Scalar::UNSIGNED, 8}};
    return u64;
  }
  const Scalar& BOOL() {
    static Scalar bool_{Scalar{"BOOL", Scalar::BOOL, 1}};
    return bool_;
  }
  const Scalar& MAGIC_ANY_TYPE() {
    static Scalar magicAnyType{Scalar{"__MAGIC_ANY_TYPE__", Scalar::MAGIC, 0}};
    return magicAnyType;
  }
  const Product& EMPTY() {
    static Product empty{{}};
    return empty;
  }

  Product::Product(std::vector<TypePtr> elements) :
    Type(nameFromTypes(elements), Category::Product), elements_(std::move(elements)) { }

  std::string Product::nameFromTypes(std::span<TypePtr const> elements) {
    return fmt::format(
      "({})", fmt::join(elements | std::ranges::views::transform([](auto const& e) { return e->name(); }), ","));
  }

}  // namespace fluir::types
