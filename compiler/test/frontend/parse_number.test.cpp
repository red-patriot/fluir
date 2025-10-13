#include <functional>
#include <string>
#include <tuple>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/frontend/text_parse.hpp"

using fluir::fe::NumberParseError;
using fluir::pt::I16;
using fluir::pt::I32;
using fluir::pt::I64;
using fluir::pt::I8;
using fluir::pt::U16;
using fluir::pt::U32;
using fluir::pt::U64;
using fluir::pt::U8;
using enum fluir::fe::NumberParseError;
using std::tuple;

template <typename T>
class TestParse : public ::testing::TestWithParam<tuple<T, std::string>> { };

#define INTEGER_PARSE_TEST(T)                                 \
  using TestParse##T = TestParse<T>;                          \
  TEST_P(TestParse##T, Test) {                                \
    auto& [expected, text] = GetParam();                      \
    auto actual = fluir::fe::parseNumber<fluir::pt::T>(text); \
    ASSERT_TRUE(actual.has_value());                          \
    EXPECT_EQ(expected, actual.value());                      \
  }

INTEGER_PARSE_TEST(I8)
INTEGER_PARSE_TEST(I16)
INTEGER_PARSE_TEST(I32)
INTEGER_PARSE_TEST(I64)
INTEGER_PARSE_TEST(U8)
INTEGER_PARSE_TEST(U16)
INTEGER_PARSE_TEST(U32)
INTEGER_PARSE_TEST(U64)

#undef INTEGER_PARSE_TEST

INSTANTIATE_TEST_SUITE_P(TestParseI8,
                         TestParseI8,
                         ::testing::Values(tuple{0, "0"},
                                           tuple{12, "12"},
                                           tuple{-43, "-43"},
                                           tuple{121, "+121"},
                                           tuple{-100, "-100"},
                                           tuple{-128, "-128"},
                                           tuple{127, "127"}));

INSTANTIATE_TEST_SUITE_P(TestParseI16,
                         TestParseI16,
                         ::testing::Values(tuple{0, "0"},
                                           tuple{24, "24"},
                                           tuple{-192, "-192"},
                                           tuple{223, "+223"},
                                           tuple{-10101, "-10101"},
                                           tuple{-32768, "-32768"},
                                           tuple{32767, "32767"}));

INSTANTIATE_TEST_SUITE_P(TestParseI32,
                         TestParseI32,
                         ::testing::Values(tuple{0, "0"},
                                           tuple{4832, "4832"},
                                           tuple{-42, "-42"},
                                           tuple{8959, "+8959"},
                                           tuple{-10101, "-10101"},
                                           tuple{-2'147'483'648, "-2147483648"},
                                           tuple{2'147'483'647, "2147483647"}));

INSTANTIATE_TEST_SUITE_P(TestParseI64,
                         TestParseI64,
                         ::testing::Values(tuple{0, "0"},
                                           tuple{24, "24"},
                                           tuple{-192, "-192"},
                                           tuple{223, "+223"},
                                           tuple{-10101, "-10101"},
                                           tuple{INT64_MIN, "-9223372036854775808"},  // -9'223'372'036'854'775808
                                           tuple{9'223'372'036'854'775'807LL, "9223372036854775807"}));
// INSTANTIATE_TEST_SUITE_P(TestParseErrorsI64,
//                          TestParseErrorsI64,
//                          ::testing::Values(tuple{CANNOT_PARSE_NUMBER, "qwerty"},
//                                            tuple{CANNOT_PARSE_NUMBER, "32j"},
//                                            tuple{CANNOT_PARSE_NUMBER, "19e7"},
//                                            tuple{CANNOT_PARSE_NUMBER, "8.11"},
//                                            tuple{CANNOT_PARSE_NUMBER, ""},
//                                            tuple{CANNOT_PARSE_NUMBER, " "},
//                                            tuple{CANNOT_PARSE_NUMBER, "."},
//                                            tuple{CANNOT_PARSE_NUMBER, "a43"},
//                                            tuple{RESULT_OUT_OF_BOUNDS, "-557111111895432452354"},
//                                            tuple{RESULT_OUT_OF_BOUNDS, "5435345245332544324349099"},
//                                            tuple{RESULT_OUT_OF_BOUNDS, "9223372036854775808"}));

INSTANTIATE_TEST_SUITE_P(TestParseU8,
                         TestParseU8,
                         ::testing::Values(tuple{0, "0"}, tuple{42, "+42"}, tuple{119, "119"}, tuple{255, "255"}));
INSTANTIATE_TEST_SUITE_P(
  TestParseU16,
  TestParseU16,
  ::testing::Values(tuple{0, "0"}, tuple{445, "+445"}, tuple{7238, "7238"}, tuple{65535, "65535"}));
INSTANTIATE_TEST_SUITE_P(TestParseU32,
                         TestParseU32,
                         ::testing::Values(tuple{0, "0"},
                                           tuple{14318, "+14318"},
                                           tuple{4648947, "4648947"},
                                           tuple{4'294'967'295, "4294967295"}));
INSTANTIATE_TEST_SUITE_P(TestParseU64,
                         TestParseU64,
                         ::testing::Values(tuple{0, "0"},
                                           tuple{71118, "+71118"},
                                           tuple{5318131, "5318131"},
                                           tuple{18'446'744'073'709'551'615ULL, "18446744073709551615"}));

class TestParseNumberErrors
  : public ::testing::TestWithParam<tuple<NumberParseError, std::function<NumberParseError()>>> { };

TEST_P(TestParseNumberErrors, Test) {
  auto& [expected, action] = GetParam();
  try {
    auto actual = action();
    EXPECT_EQ(expected, actual);
  } catch (const std::exception& e) {
    FAIL() << "FAILED WITH EXCEPTION: " << e.what();
  }
}

template <typename T>
const auto doParse = [](std::string_view text) {
  return std::function([text]() {
    auto result = fluir::fe::parseNumber<T>(text);
    return result.error();
  });
};

INSTANTIATE_TEST_SUITE_P(TestParseNumberErrors,
                         TestParseNumberErrors,
                         ::testing::Values(tuple{CANNOT_PARSE_NUMBER, doParse<I8>("asdf")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<I8>("12b")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<I8>("19e7")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<I8>("8.11")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<I8>("")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<I8>(" ")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<I8>(".")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<I8>("a43")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<I8>("12345")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<I8>("-490")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<I8>("2345")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<I8>("128")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<I16>("32768")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<I16>("43923498389247")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<I16>("-43253")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<I32>("2147483648")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<I32>("994730274739242349")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<I32>("-4233234233")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<I64>("9223372036854775808")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<I64>("82135181218008481048091508940")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<I64>("-43235335255434556440")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<U8>("asdf")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<U8>("12b")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<U8>("19e7")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<U8>("8.11")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<U8>("-432")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<U8>("")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<U8>(" ")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<U8>(".")},
                                           tuple{CANNOT_PARSE_NUMBER, doParse<U8>("a43")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<U8>("18915")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<U8>("256")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<U16>("65536")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<U16>("71681805181")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<U32>("4294967296")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<U32>("816841981871")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<U64>("18446744073709551616")},
                                           tuple{RESULT_OUT_OF_BOUNDS, doParse<U64>("971289402871054907809870")}));
