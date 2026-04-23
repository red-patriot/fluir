#include "compiler/utility/diagnostic/pretty_msg.hpp"

#include <format>
#include <tuple>

#include <gtest/gtest.h>

namespace fd = fluir::diagnostic;
using std::tuple;

class TestPrettyDiagnosticMessage : public ::testing::TestWithParam<tuple<std::string, fd::Code>> { };

TEST_P(TestPrettyDiagnosticMessage, Test) {
  const auto& [expected, code] = GetParam();

  const auto actual = fd::prettyMessage(code);

  EXPECT_EQ(expected, actual);
}

INSTANTIATE_TEST_SUITE_P(TestPrettyDiagnosticMessage,
                         TestPrettyDiagnosticMessage,
                         ::testing::Values(tuple{"An unknown error was emitted.", fd::Code::GENERIC_ERROR},
                                           tuple{"An unknown warning was emitted.", fd::Code::GENERIC_WARNING},
                                           tuple{"", fd::Code::GENERIC_NOTE},
                                           tuple{"ERROR 0x8001", static_cast<fd::Code>(0x8001)}));

class TestAllDiagnosticsHaveAPrettyMessage : public ::testing::TestWithParam<fd::Code> { };

TEST_P(TestAllDiagnosticsHaveAPrettyMessage, Test) {
  const auto& code = GetParam();

  auto message = fd::prettyMessage(code);

  EXPECT_FALSE(message.starts_with("ERROR 0x"));
}

#define FLUIR_ENUMERATE(x) fd::Code::x,

const static std::vector<fd::Code> codes{FLUIR_DIAGNOSTIC_CODE(FLUIR_ENUMERATE)};

INSTANTIATE_TEST_SUITE_P(TestAllDiagnosticsHaveAPrettyMessage,
                         TestAllDiagnosticsHaveAPrettyMessage,
                         ::testing::ValuesIn(codes));

#undef FLUIR_ENUMERATE
