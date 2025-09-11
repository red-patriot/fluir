#ifndef FLUIR_VALUE_H
#define FLUIR_VALUE_H

#include <cstdint>
#include <stdexcept>

namespace fluir::code {
  enum class ValueType {
    F64,
    I64,
    U64,
  };

  /* A generic Fluir value */
  class Value {
   public:
    explicit Value(double d) : type_{ValueType::F64}, data_{d} { }
    explicit Value(std::int64_t i) : type_{ValueType::I64}, data_{i} { }
    explicit Value(std::uint64_t u) : type_{ValueType::U64}, data_{u} { }

    [[nodiscard]] ValueType type() const { return type_; };

    [[nodiscard]] double& asF64() {
      assertType(ValueType::F64);
      return data_.f64;
    }
    [[nodiscard]] const double& asF64() const {
      assertType(ValueType::F64);
      return data_.f64;
    }

    [[nodiscard]] std::int64_t& asI64() {
      assertType(ValueType::I64);
      return data_.i64;
    }
    [[nodiscard]] const std::int64_t& asI64() const {
      assertType(ValueType::I64);
      return data_.i64;
    }

    [[nodiscard]] std::uint64_t& asU64() {
      assertType(ValueType::U64);
      return data_.u64;
    }
    [[nodiscard]] const std::uint64_t& asU64() const {
      assertType(ValueType::U64);
      return data_.u64;
    }

   private:
    ValueType type_;

    union Data {
      double f64;
      std::int64_t i64;
      std::uint64_t u64;

      explicit(false) Data(double d) : f64(d) { }
      explicit(false) Data(std::int64_t i) : i64(i) { }
      explicit(false) Data(std::uint64_t u) : u64(u) { }
    } data_;

    void assertType(ValueType type) const {
      if (type_ != type) {
        throw std::runtime_error("Actual value type does not match");
      }
    }
  };

  namespace value_literals {
    inline `Value operator""_f64(long double d) { return Value{static_cast<double>(d)}; }
    inline Value operator""_i64(unsigned long long i) { return Value{static_cast<std::int64_t>(i)}; }
    inline Value operator""_u64(unsigned long long int i) { return Value{static_cast<std::uint64_t>(i)}; }

  }  // namespace value_literals
}  // namespace fluir::code

#endif  // FLUIR_VALUE_H
