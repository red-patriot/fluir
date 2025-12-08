#ifndef FLUIR_VALUE_H
#define FLUIR_VALUE_H

#include <stdexcept>

#include "primitives.hpp"

namespace fluir::code {

  /* A generic Fluir value */
  class Value {
    friend bool operator==(const Value&, const Value&);

   public:
#define FLUIR_VALUE_CONSTRUCTOR(Type, Concrete) \
  explicit Value(Concrete d) : type_{PrimitiveType::Type}, data_(d) { }

    FLUIR_CODE_PRIMITIVE_TYPES(FLUIR_VALUE_CONSTRUCTOR)

#undef FLUIR_VALUE_CONSTRUCTOR

    [[nodiscard]] PrimitiveType type() const { return type_; };

#define TEMP_CONCAT(a, b) a##b
#define FLUIR_VALUE_ACCESSOR(Type, Concrete)                    \
  [[nodiscard]] Concrete& TEMP_CONCAT(as, Type)() {             \
    assertType(PrimitiveType::Type);                            \
    return data_.Type;                                          \
  }                                                             \
  [[nodiscard]] const Concrete& TEMP_CONCAT(as, Type)() const { \
    assertType(PrimitiveType::Type);                            \
    return data_.Type;                                          \
  }

    FLUIR_CODE_PRIMITIVE_TYPES(FLUIR_VALUE_ACCESSOR)

#undef FLUIR_VALUE_ACCESSOR

   private:
    PrimitiveType type_;

    union Data {
#define FLUIR_VALUE_UNION_MEMBER(Type, Concrete) \
  Concrete Type;                                 \
  explicit(false) Data(Concrete d) : Type(d) { }

      FLUIR_CODE_PRIMITIVE_TYPES(FLUIR_VALUE_UNION_MEMBER)
#undef FLUIR_VALUE_UNION_MEMBER
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
#define FLUIR_VALUE_COMPARE(Type, Concrete) \
  case PrimitiveType::Type:                 \
    return TEMP_CONCAT(lhs.as, Type)() == TEMP_CONCAT(rhs.as, Type)();

      FLUIR_CODE_PRIMITIVE_TYPES(FLUIR_VALUE_COMPARE)

#undef FLUIR_VALUE_COMPARE
#undef TEMP_CONCAT
    }
    return false;
  }
}  // namespace fluir::code

#endif  // FLUIR_VALUE_H
