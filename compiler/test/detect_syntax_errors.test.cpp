#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "compiler/frontend/parser.hpp"

namespace fs = std::filesystem;

class TestDetectSyntaxError : public ::testing::TestWithParam<fs::path> {
 public:
  std::string readErrors(const fs::path& file) {
    std::fstream fin(file);
    std::stringstream ss;
    ss << fin.rdbuf();
    return ss.str();
  }
};

TEST_P(TestDetectSyntaxError, Test) {
  const auto programFile = GetParam();
  const auto errorsFile = fs::path{programFile}.replace_extension(".errors");
  const auto errors = readErrors(errorsFile);

  fluir::Parser uut;

  auto results = uut.parseFile(programFile);

  std::stringstream ss;
  for (const auto& diagnostic : results.diagnostics()) {
    ss << fluir::toString(diagnostic) << '\n';
  }

  auto actual = ss.str();

  EXPECT_EQ(errors, actual);
}

static std::vector<fs::path> getTestPrograms(const fs::path& parent) {
  std::vector<fs::path> programs;

  for (const auto& entry : fs::directory_iterator(parent)) {
    if (fs::is_regular_file(entry) && fs::path(entry).extension() == ".fl") {
      programs.emplace_back(entry.path());
    }
  }

  return programs;
}

INSTANTIATE_TEST_SUITE_P(TestDetectSyntaxError, TestDetectSyntaxError,
                         ::testing::ValuesIn(getTestPrograms(TEST_FOLDER / "programs" / "syntax_error")),
                         [](const ::testing::TestParamInfo<fs::path>& info) {
                           return info.param.stem().string();
                         });
