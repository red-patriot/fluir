#include "compiler/utility/results.hpp"

#include <string>
#include <type_traits>

#include <gtest/gtest.h>

TEST(TestResults, Typedefs) {
  using ResultsType = fluir::Results<int>;

  EXPECT_TRUE((std::is_same_v<int, ResultsType::ValueType>));
  EXPECT_TRUE((std::is_same_v<fluir::Diagnostics, ResultsType::DiagsType>));
}

TEST(TestResults, ConstructAndGetValue) {
  std::string expected = "there is a value here!";

  fluir::Results<std::string> actual{expected};

  EXPECT_TRUE(actual.hasValue());
  EXPECT_EQ(expected, actual.value());
}

TEST(TestResults, ConstructWithErrors) {
  fluir::Diagnostics error;
  error.emplace_back(fluir::Diagnostic{fluir::Diagnostic::ERROR, ""});
  fluir::Results<int> actual{std::move(error)};

  EXPECT_FALSE(actual.hasValue());
  EXPECT_EQ(std::nullopt, actual.optionalValue());
  EXPECT_TRUE(actual.containsErrors());
}

TEST(TestResults, GetDiagnostics) {
  fluir::Diagnostics error{fluir::Diagnostic{fluir::Diagnostic::ERROR, ""}};
  fluir::Results<int> actual{error};

  EXPECT_EQ(error, actual.diagnostics());
}

TEST(TestResults, ConstructWithBoth) {
  fluir::Results<int> actual{12, {fluir::Diagnostic{fluir::Diagnostic::WARNING, ""}}};

  EXPECT_TRUE(actual.hasValue());
  EXPECT_EQ(12, actual.value());
  EXPECT_TRUE(actual.hasDiagnostics());
  EXPECT_FALSE(actual.containsErrors());
}

TEST(TestResults, DereferenceOperator) {
  std::string expected = "hello there!";
  fluir::Results<std::string> actual{"hello there!"};

  EXPECT_EQ(expected, *actual);
  EXPECT_STRCASEEQ(expected.c_str(), actual->c_str());
}

TEST(TestResults, CopyConstructorWithValue) {
  fluir::Results<std::string> first{"hello there!"};
  fluir::Results<std::string> second = first;

  EXPECT_EQ(first.value(), second.value());
  EXPECT_EQ(first.diagnostics(), second.diagnostics());
}

TEST(TestResults, CopyConstructorWithDiagnostics) {
  fluir::Results<std::string> first{{fluir::Diagnostic{fluir::Diagnostic::NOTE, ""}}};
  fluir::Results<std::string> second = first;

  EXPECT_EQ(first.hasValue(), second.hasValue());
  EXPECT_EQ(first.diagnostics(), second.diagnostics());
}

TEST(TestResults, MoveConstructorWithValue) {
  fluir::Results<std::string> first{"hello there!"};
  fluir::Results<std::string> second = std::move(first);

  EXPECT_FALSE(first.hasValue());
  EXPECT_FALSE(first.hasDiagnostics());

  EXPECT_EQ("hello there!", second.value());
  EXPECT_FALSE(second.hasDiagnostics());
}

TEST(TestResults, MoveConstructorWithDiagnostics) {
  fluir::Diagnostics expected{
      fluir::Diagnostic{fluir::Diagnostic::NOTE, ""}};
  fluir::Results<std::string> first{{fluir::Diagnostic{fluir::Diagnostic::NOTE, ""}}};
  fluir::Results<std::string> second = std::move(first);

  EXPECT_FALSE(first.hasValue());
  EXPECT_FALSE(first.hasDiagnostics());

  EXPECT_FALSE(second.hasValue());
  EXPECT_EQ(expected, second.diagnostics());
}

TEST(TestResults, CopyAssignWithValue) {
  fluir::Results<std::string> first{"hello there!"};
  fluir::Results<std::string> second{{fluir::Diagnostic{fluir::Diagnostic::ERROR, ""}}};
  second = first;

  EXPECT_EQ(first.value(), second.value());
  EXPECT_EQ(first.diagnostics(), second.diagnostics());
}

TEST(TestResults, CopyAssignWithDiagnostics) {
  fluir::Results<std::string> first{{fluir::Diagnostic{fluir::Diagnostic::NOTE, ""}}};
  fluir::Results<std::string> second{"no problem here"};
  second = first;

  EXPECT_EQ(first.hasValue(), second.hasValue());
  EXPECT_EQ(first.diagnostics(), second.diagnostics());
}

TEST(TestResults, MoveAssignWithValue) {
  fluir::Results<std::string> first{"hello there!"};
  fluir::Results<std::string> second{{fluir::Diagnostic{fluir::Diagnostic::ERROR, ""}}};
  second = std::move(first);

  EXPECT_FALSE(first.hasValue());
  EXPECT_FALSE(first.hasDiagnostics());

  EXPECT_EQ("hello there!", second.value());
  EXPECT_FALSE(second.hasDiagnostics());
}

TEST(TestResults, MoveAssignWithDiagnostics) {
  fluir::Diagnostics expected{
      fluir::Diagnostic{fluir::Diagnostic::NOTE, ""}};
  fluir::Results<std::string> first{{fluir::Diagnostic{fluir::Diagnostic::NOTE, ""}}};
  fluir::Results<std::string> second{"no problem here"};
  second = std::move(first);

  EXPECT_FALSE(first.hasValue());
  EXPECT_FALSE(first.hasDiagnostics());

  EXPECT_FALSE(second.hasValue());
  EXPECT_EQ(expected, second.diagnostics());
}
