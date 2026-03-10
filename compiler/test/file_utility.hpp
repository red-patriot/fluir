#ifndef FLUIR_COMPILER_TEST_FILE_UTILITY_HPP
#define FLUIR_COMPILER_TEST_FILE_UTILITY_HPP

#include <filesystem>
#include <fstream>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/utility/diagnostic/code.hpp"
#include "fluir/testing/test_files_dir.hpp"

namespace fluir::test {
  inline std::string readContents(const std::filesystem::path& file) {
    std::fstream fin(file);
    std::stringstream ss;
    ss << fin.rdbuf();
    return ss.str();
  }

  inline std::filesystem::path getGoldenFile(const std::filesystem::path& absoluteCode,
                                             std::string_view errorsExtension) {
    static const std::filesystem::path PROGRAMS_ERRORS_DIR = std::filesystem::path{ROOT_COMPILER_TEST_DIR} / "programs";
    auto relativeCodePath = fluir::test::getRelativePath(absoluteCode);
    auto errorsPath = relativeCodePath.replace_extension(errorsExtension);
    auto absErrorsPath = PROGRAMS_ERRORS_DIR / errorsPath;
    return absErrorsPath;
  }

  inline std::vector<diagnostic::Code> getErrors(const std::filesystem::path& file) {
    static const std::unordered_map<std::string, diagnostic::Code> translation{
#define FLUIR_TEST_MAKE_ERRORS_MAP(code) {#code, diagnostic::Code::code},
      FLUIR_DIAGNOSTIC_CODE(FLUIR_TEST_MAKE_ERRORS_MAP)
#undef FLUIR_TEST_MAKE_ERRORS_MAP
    };

    std::vector<diagnostic::Code> errors;
    std::ifstream fin(file);
    for (std::string line; std::getline(fin, line);) {
      if (translation.contains(line)) {
        errors.push_back(translation.at(line));
      } else {
        errors.push_back(diagnostic::Code::GENERIC_ERROR);
      }
    }
    return errors;
  }

  inline std::string filePathName(const ::testing::TestParamInfo<std::filesystem::path>& info) {
    return info.param.stem().string();
  }
}  // namespace fluir::test

#endif
