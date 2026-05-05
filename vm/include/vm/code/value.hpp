#ifndef FLUIR_VM_CODE_VALUE_H
#define FLUIR_VM_CODE_VALUE_H

#include <stdexcept>

#include <bytecode/primitives.hpp>

namespace fluir::code {

  /* A generic Fluir value */
  class Value {
    friend bool operator==(const Value&, const Value&);

   public:
    Value() : type_{PrimitiveType::EMPTY} { }
    explicit Value(double d) : type_{PrimitiveType::F64}, data_(d) { }
    explicit Value(std::int8_t d) : type_{PrimitiveType::I8}, data_(d) { }
    explicit Value(std::int16_t d) : type_{PrimitiveType::I16}, data_(d) { }
    explicit Value(std::int32_t d) : type_{PrimitiveType::I32}, data_(d) { }
    explicit Value(std::int64_t d) : type_{PrimitiveType::I64}, data_(d) { }
    explicit Value(std::uint8_t d) : type_{PrimitiveType::U8}, data_(d) { }
    explicit Value(std::uint16_t d) : type_{PrimitiveType::U16}, data_(d) { }
    explicit Value(std::uint32_t d) : type_{PrimitiveType::U32}, data_(d) { }
    explicit Value(std::uint64_t d) : type_{PrimitiveType::U64}, data_(d) { }

    [[nodiscard]] PrimitiveType type() const { return type_; }
    [[nodiscard]] bool empty() const { return type_ == PrimitiveType::EMPTY; }

    [[nodiscard]] double& asF64() {
      assertType(PrimitiveType::F64);
      return data_.F64;
    }
    [[nodiscard]] const double& asF64() const {
      assertType(PrimitiveType::F64);
      return data_.F64;
    }

    [[nodiscard]] std::int8_t& asI8() {
      assertType(PrimitiveType::I8);
      return data_.I8;
    }
    [[nodiscard]] const std::int8_t& asI8() const {
      assertType(PrimitiveType::I8);
      return data_.I8;
    }

    [[nodiscard]] std::int16_t& asI16() {
      assertType(PrimitiveType::I16);
      return data_.I16;
    }
    [[nodiscard]] const std::int16_t& asI16() const {
      assertType(PrimitiveType::I16);
      return data_.I16;
    }

    [[nodiscard]] std::int32_t& asI32() {
      assertType(PrimitiveType::I32);
      return data_.I32;
    }
    [[nodiscard]] const std::int32_t& asI32() const {
      assertType(PrimitiveType::I32);
      return data_.I32;
    }

    [[nodiscard]] std::int64_t& asI64() {
      assertType(PrimitiveType::I64);
      return data_.I64;
    }
    [[nodiscard]] const std::int64_t& asI64() const {
      assertType(PrimitiveType::I64);
      return data_.I64;
    }

    [[nodiscard]] std::uint8_t& asU8() {
      assertType(PrimitiveType::U8);
      return data_.U8;
    }
    [[nodiscard]] const std::uint8_t& asU8() const {
      assertType(PrimitiveType::U8);
      return data_.U8;
    }

    [[nodiscard]] std::uint16_t& asU16() {
      assertType(PrimitiveType::U16);
      return data_.U16;
    }
    [[nodiscard]] const std::uint16_t& asU16() const {
      assertType(PrimitiveType::U16);
      return data_.U16;
    }

    [[nodiscard]] std::uint32_t& asU32() {
      assertType(PrimitiveType::U32);
      return data_.U32;
    }
    [[nodiscard]] const std::uint32_t& asU32() const {
      assertType(PrimitiveType::U32);
      return data_.U32;
    }

    [[nodiscard]] std::uint64_t& asU64() {
      assertType(PrimitiveType::U64);
      return data_.U64;
    }
    [[nodiscard]] const std::uint64_t& asU64() const {
      assertType(PrimitiveType::U64);
      return data_.U64;
    }

   private:
    PrimitiveType type_;

    union Data {
      double F64;
      std::int8_t I8;
      std::int16_t I16;
      std::int32_t I32;
      std::int64_t I64;
      std::uint8_t U8;
      std::uint16_t U16;
      std::uint32_t U32;
      std::uint64_t U64;

      Data() : I64(0) { /* Zero-initialize if EMPTY */ }
      explicit(false) Data(double d) : F64(d) { }
      explicit(false) Data(std::int8_t d) : I8(d) { }
      explicit(false) Data(std::int16_t d) : I16(d) { }
      explicit(false) Data(std::int32_t d) : I32(d) { }
      explicit(false) Data(std::int64_t d) : I64(d) { }
      explicit(false) Data(std::uint8_t d) : U8(d) { }
      explicit(false) Data(std::uint16_t d) : U16(d) { }
      explicit(false) Data(std::uint32_t d) : U32(d) { }
      explicit(false) Data(std::uint64_t d) : U64(d) { }
    } data_;

    void assertType(PrimitiveType type) const {
      if (type_ != type) {
        throw std::runtime_error("Actual value type does not match");
      }
    }
  };

  namespace value_literals {
    inline Value operator""_f64(long double d) { return Value{static_cast<double>(d)}; }
    inline Value operator""_i64(unsigned long long i) { return Value{static_cast<std::int64_t>(i)}; }
    inline Value operator""_i32(unsigned long long i) { return Value{static_cast<std::int32_t>(i)}; }
    inline Value operator""_i16(unsigned long long i) { return Value{static_cast<std::int16_t>(i)}; }
    inline Value operator""_i8(unsigned long long i) { return Value{static_cast<std::int8_t>(i)}; }
    inline Value operator""_u64(unsigned long long int u) { return Value{static_cast<std::uint64_t>(u)}; }
    inline Value operator""_u32(unsigned long long int u) { return Value{static_cast<std::uint32_t>(u)}; }
    inline Value operator""_u16(unsigned long long int u) { return Value{static_cast<std::uint16_t>(u)}; }
    inline Value operator""_u8(unsigned long long int u) { return Value{static_cast<std::uint8_t>(u)}; }
  }  // namespace value_literals

  inline bool operator==(const Value& lhs, const Value& rhs) {
    if (lhs.type() != rhs.type()) {
      return false;
    }

    switch (lhs.type()) {
      case PrimitiveType::EMPTY:
        return true;  // All empty values are mutually equal
      case PrimitiveType::F64:
        return lhs.asF64() == rhs.asF64();
      case PrimitiveType::I8:
        return lhs.asI8() == rhs.asI8();
      case PrimitiveType::I16:
        return lhs.asI16() == rhs.asI16();
      case PrimitiveType::I32:
        return lhs.asI32() == rhs.asI32();
      case PrimitiveType::I64:
        return lhs.asI64() == rhs.asI64();
      case PrimitiveType::U8:
        return lhs.asU8() == rhs.asU8();
      case PrimitiveType::U16:
        return lhs.asU16() == rhs.asU16();
      case PrimitiveType::U32:
        return lhs.asU32() == rhs.asU32();
      case PrimitiveType::U64:
        return lhs.asU64() == rhs.asU64();
    }
    return false;
  }
}  // namespace fluir::code

#endif  // FLUIR_VALUE_H
