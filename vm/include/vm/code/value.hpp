#ifndef FLUIR_VM_CODE_VALUE_H
#define FLUIR_VM_CODE_VALUE_H

#include <stdexcept>

#include <bytecode/primitives.hpp>

#include "vm/code/string.hpp"

namespace fluir::code {

  /* A generic Fluir value */
  class Value {
    friend bool operator==(const Value&, const Value&);

   public:
    Value() : type_{PrimitiveType::EMPTY}, u64_{/*Just zero-out all the data*/} { }
    explicit Value(double d) : type_{PrimitiveType::F64}, f64_(d) { }
    explicit Value(std::int8_t d) : type_{PrimitiveType::I8}, i8_(d) { }
    explicit Value(std::int16_t d) : type_{PrimitiveType::I16}, i16_(d) { }
    explicit Value(std::int32_t d) : type_{PrimitiveType::I32}, i32_(d) { }
    explicit Value(std::int64_t d) : type_{PrimitiveType::I64}, i64_(d) { }
    explicit Value(std::uint8_t d) : type_{PrimitiveType::U8}, u8_(d) { }
    explicit Value(std::uint16_t d) : type_{PrimitiveType::U16}, u16_(d) { }
    explicit Value(std::uint32_t d) : type_{PrimitiveType::U32}, u32_(d) { }
    explicit Value(std::uint64_t d) : type_{PrimitiveType::U64}, u64_(d) { }
    explicit Value(String s) : type_{PrimitiveType::STR}, str_(std::move(s)) { }
    explicit Value(bool b) : type_{PrimitiveType::BOOL}, bool_(b) { }

    Value(const Value& other) : type_(other.type_) {
      if (type_ == PrimitiveType::STR) {
        new (&str_) String(other.str_);
      } else {
        memcpy(&u64_, &other.u64_, sizeof(u64_));
      }
    }

    Value& operator=(const Value& other) {
      if (this == &other) {
        return *this;
      }
      if (type_ == PrimitiveType::STR) {
        str_.~String();
      }
      type_ = other.type_;
      if (type_ == PrimitiveType::STR) {
        new (&str_) String(other.str_);
      } else {
        memcpy(&u64_, &other.u64_, sizeof(u64_));
      }
      return *this;
    }

    Value(Value&& other) : type_(other.type_) {
      if (type_ == PrimitiveType::STR) {
        new (&str_) String(std::move(other.str_));
      } else {
        memcpy(&u64_, &other.u64_, sizeof(u64_));
      }
      other.type_ = PrimitiveType::EMPTY;
    }
    Value& operator=(Value&& other) {
      if (this == &other) {
        return *this;
      }
      if (type_ == PrimitiveType::STR) {
        str_.~String();
      }
      type_ = other.type_;
      if (type_ == PrimitiveType::STR) {
        new (&str_) String(std::move(other.str_));
      } else {
        memcpy(&u64_, &other.u64_, sizeof(u64_));
      }
      other.type_ = PrimitiveType::EMPTY;
      return *this;
    }
    ~Value() {
      if (type_ == PrimitiveType::STR) {
        // String has a non-trivial destructor, so call it explicitly here
        str_.~String();
      }
    }

    [[nodiscard]] PrimitiveType type() const { return type_; }
    [[nodiscard]] bool empty() const { return type_ == PrimitiveType::EMPTY; }

    [[nodiscard]] double& asF64() {
      assertType(PrimitiveType::F64);
      return f64_;
    }
    [[nodiscard]] const double& asF64() const {
      assertType(PrimitiveType::F64);
      return f64_;
    }

    [[nodiscard]] std::int8_t& asI8() {
      assertType(PrimitiveType::I8);
      return i8_;
    }
    [[nodiscard]] const std::int8_t& asI8() const {
      assertType(PrimitiveType::I8);
      return i8_;
    }

    [[nodiscard]] std::int16_t& asI16() {
      assertType(PrimitiveType::I16);
      return i16_;
    }
    [[nodiscard]] const std::int16_t& asI16() const {
      assertType(PrimitiveType::I16);
      return i16_;
    }

    [[nodiscard]] std::int32_t& asI32() {
      assertType(PrimitiveType::I32);
      return i32_;
    }
    [[nodiscard]] const std::int32_t& asI32() const {
      assertType(PrimitiveType::I32);
      return i32_;
    }

    [[nodiscard]] std::int64_t& asI64() {
      assertType(PrimitiveType::I64);
      return i64_;
    }
    [[nodiscard]] const std::int64_t& asI64() const {
      assertType(PrimitiveType::I64);
      return i64_;
    }

    [[nodiscard]] std::uint8_t& asU8() {
      assertType(PrimitiveType::U8);
      return u8_;
    }
    [[nodiscard]] const std::uint8_t& asU8() const {
      assertType(PrimitiveType::U8);
      return u8_;
    }

    [[nodiscard]] std::uint16_t& asU16() {
      assertType(PrimitiveType::U16);
      return u16_;
    }
    [[nodiscard]] const std::uint16_t& asU16() const {
      assertType(PrimitiveType::U16);
      return u16_;
    }

    [[nodiscard]] std::uint32_t& asU32() {
      assertType(PrimitiveType::U32);
      return u32_;
    }
    [[nodiscard]] const std::uint32_t& asU32() const {
      assertType(PrimitiveType::U32);
      return u32_;
    }

    [[nodiscard]] std::uint64_t& asU64() {
      assertType(PrimitiveType::U64);
      return u64_;
    }
    [[nodiscard]] const std::uint64_t& asU64() const {
      assertType(PrimitiveType::U64);
      return u64_;
    }

    [[nodiscard]] String& asStr() {
      assertType(PrimitiveType::STR);
      return str_;
    }
    [[nodiscard]] const String& asStr() const {
      assertType(PrimitiveType::STR);
      return str_;
    }

    [[nodiscard]] bool& asBool() {
      assertType(PrimitiveType::BOOL);
      return bool_;
    }
    [[nodiscard]] const bool& asBool() const {
      assertType(PrimitiveType::BOOL);
      return bool_;
    }

   private:
    PrimitiveType type_{PrimitiveType::EMPTY};

    // Use a union here over a std::variant because profiling shows it to be
    // too slow to be convenient. Raw union avoids expensive ops in std::get
    // and extra checking because we already track the active member (type_)
    union {
      F64 f64_;
      I8 i8_;
      I16 i16_;
      I32 i32_;
      I64 i64_;
      U8 u8_;
      U16 u16_;
      U32 u32_;
      U64 u64_;
      String str_;
      bool bool_;
    };

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

    inline const Value TRUE_VALUE{true};
    inline const Value FALSE_VALUE{false};
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
      case PrimitiveType::STR:
        return lhs.asStr() == rhs.asStr();
      case PrimitiveType::BOOL:
        return lhs.asBool() == rhs.asBool();
    }
    return false;
  }

  Value staticString(std::string_view s);
}  // namespace fluir::code

#endif  // FLUIR_VALUE_H
