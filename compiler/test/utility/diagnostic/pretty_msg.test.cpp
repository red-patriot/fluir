#include "compiler/utility/diagnostic/pretty_msg.hpp"

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
