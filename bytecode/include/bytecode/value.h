#ifndef FLUIR_VALUE_H
#define FLUIR_VALUE_H

namespace fluir::code {
  enum class ValueType {
    FLOAT64,
  };

  /* A generic Fluir value */
  class Value {
   public:
    /* implicit */ Value(double d) : data_(d) { }

    double& asFloat64() { return data_; }
    const double& asFloat64() const { return data_; }

    ValueType type() const { return ValueType::FLOAT64; }

   private:
    double data_;
  };
}  // namespace fluir::code

#endif  // FLUIR_VALUE_H
