#ifndef FLUIR_VALUE_H
#define FLUIR_VALUE_H

#include <cstdint>
#include <stdexcept>

namespace fluir::code {
  // clang-format off
#define FLUIR_CODE_VALUE_TYPES(code)  \
  code(F64, double)                   \
  code(I64, std::int64_t)             \
  code(U64, std::uint64_t)
  // clang-format on

#define enumerate(Type, ConcreteType) Type,
  enum class ValueType { FLUIR_CODE_VALUE_TYPES(enumerate) };
#undef enumerate

  /* A generic Fluir value */
  class Value {
   public:
#define FLUIR_VALUE_CONSTRUCTOR(Type, Concrete) \
  explicit Value(Concrete d) : type_{ValueType::Type}, data_(d) { }

    FLUIR_CODE_VALUE_TYPES(FLUIR_VALUE_CONSTRUCTOR)

#undef FLUIR_VALUE_CONSTRUCTOR

    [[nodiscard]] ValueType type() const { return type_; };

#define TEMP_CONCAT(a, b) a##b
#define FLUIR_VALUE_ACCESSOR(Type, Concrete)                    \
  [[nodiscard]] Concrete& TEMP_CONCAT(as, Type)() {             \
    assertType(ValueType::Type);                                \
    return data_.Type;                                          \
  }                                                             \
  [[nodiscard]] const Concrete& TEMP_CONCAT(as, Type)() const { \
    assertType(ValueType::Type);                                \
    return data_.Type;                                          \
  }

    FLUIR_CODE_VALUE_TYPES(FLUIR_VALUE_ACCESSOR)

#undef FLUIR_VALUE_ACCESSOR
#undef TEMP_CONCAT

   private:
    ValueType type_;

    union Data {
#define FLUIR_VALUE_UNION_MEMBER(Type, Concrete) \
  Concrete Type;                                 \
  explicit(false) Data(Concrete d) : Type(d) { }

      FLUIR_CODE_VALUE_TYPES(FLUIR_VALUE_UNION_MEMBER)
#undef FLUIR_VALUE_UNION_MEMBER
    } data_;

    void assertType(ValueType type) const {
      if (type_ != type) {
        throw std::runtime_error("Actual value type does not match");
      }
    }
  };

  namespace value_literals {
    inline Value operator""_f64(long double d) { return Value{static_cast<double>(d)}; }
    inline Value operator""_i64(unsigned long long i) { return Value{static_cast<std::int64_t>(i)}; }
    inline Value operator""_u64(unsigned long long int i) { return Value{static_cast<std::uint64_t>(i)}; }

  }  // namespace value_literals
}  // namespace fluir::code

#endif  // FLUIR_VALUE_H
