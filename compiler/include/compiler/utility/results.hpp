#ifndef FLUIR_COMPILER_UTILITY_RESULTS_HPP
#define FLUIR_COMPILER_UTILITY_RESULTS_HPP

#include <optional>

#include "diagnostics.hpp"

namespace fluir {
  /** Contains data and accompanying diagnostics.
   * The data may or may not exist, depending on what diagnostics are produced
   * and if the diagnostics represent an error. It is valid to provide both
   * a value and errors.
   */
  template <typename T>
  class Results {
   public:
    using ValueType = T;
    using DiagsType = Diagnostics;

    Results() = default;
    explicit Results(const ValueType& value) :
        value_(value), diagnostics_() { }
    explicit Results(const DiagsType& diagnostics) :
        value_(std::nullopt), diagnostics_(diagnostics) { }
    Results(const ValueType& value, const DiagsType& diagnostics) :
        value_(value), diagnostics_(diagnostics) { }

    Results(const Results&) = default;
    Results& operator=(const Results&) = default;

    Results(Results&& other) :
        value_(std::move(other.value_)),
        diagnostics_(std::move(other.diagnostics_)) {
      other.value_ = std::nullopt;
      other.diagnostics_ = Diagnostics{};
    }
    Results& operator=(Results&& other) {
      if (&other != this) {
        value_ = std::move(other.value_);
        diagnostics_ = std::move(other.diagnostics_);
        other.value_ = std::nullopt;
        other.diagnostics_ = Diagnostics{};
      }
      return *this;
    }
    ~Results() = default;

    bool hasValue() const { return value_.has_value(); }
    bool containsErrors() const { return diagnostics_.containsErrors(); }
    bool hasDiagnostics() const { return !diagnostics_.empty(); }

    const std::optional<ValueType>& optionalValue() const { return value_; }
    const ValueType& value() const { return value_.value(); }
    ValueType& value() { return value_.value(); }

    const DiagsType& diagnostics() const { return diagnostics_; }
    DiagsType& diagnostics() { return diagnostics_; }

    const ValueType& operator*() const { }
    ValueType& operator*() { return value_.value(); }

    const ValueType* operator->() const { return value_.operator->(); }
    ValueType* operator->() { return value_.operator->(); }

   private:
    std::optional<ValueType> value_; /**< The (optional) value produced by an operation */
    DiagsType diagnostics_;          /**< Diagnostics which accompany the production of value_ */
  };
};  // namespace fluir

#endif
