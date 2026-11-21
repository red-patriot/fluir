#ifndef FLUIR_COMPILER_TEST_FILE_UTILITY_HPP
#define FLUIR_COMPILER_TEST_FILE_UTILITY_HPP

#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/utility/diagnostic/code.hpp"

namespace fluir::test {
  inline std::string readContents(const std::filesystem::path& file) {
    std::fstream fin(file);
    std::stringstream ss;
    ss << fin.rdbuf();
    return ss.str();
  }

  inline std::unordered_multimap<diagnostic::Code, fluir::FullID> getErrors(const std::filesystem::path& file) {
    // TODO: Add mapping to expected elements
    static const std::unordered_map<std::string, diagnostic::Code> translation{
#define FLUIR_TEST_MAKE_ERRORS_MAP(code) {#code, diagnostic::Code::code},
      FLUIR_DIAGNOSTIC_CODE(FLUIR_TEST_MAKE_ERRORS_MAP)
#undef FLUIR_TEST_MAKE_ERRORS_MAP
    };

    std::unordered_multimap<diagnostic::Code, fluir::FullID> errors;
    std::ifstream fin(file);

    for (std::string line; std::getline(fin, line);) {
      auto codeName = line.substr(0, line.find(' '));
      auto code = diagnostic::Code::GENERIC_ERROR;
      if (translation.contains(codeName)) {
        code = translation.at(codeName);
      }
      FullID fullId;
      auto i = codeName.size() + 1;
      if (i >= line.size()) {
        errors.emplace(code, std::move(fullId));
        continue;  // These errors don't give an ID right now (probably in error?)
      }
      for (std::string idStr = line.substr(i, line.find(' ', i)); !idStr.empty();) {
        ID id{INVALID_ID};
        auto result = std::from_chars(idStr.data(), idStr.data() + idStr.size(), id);
        fullId.emplace_back(id);
        auto numLen = result.ptr - idStr.data();
        auto spaceAfter = idStr.find(' ', numLen);
        if (spaceAfter < idStr.size()) {
          auto next = idStr.substr(spaceAfter + 1);
          idStr = next;
        } else {
          // Nothing else to parse
          break;
        }
      }
      errors.emplace(code, std::move(fullId));
    }
    return errors;
  }

  inline std::unordered_multimap<diagnostic::Code, int> getLineErrors(const std::filesystem::path& file) {
    // TODO: Add mapping to expected elements
    static const std::unordered_map<std::string, diagnostic::Code> translation{
#define FLUIR_TEST_MAKE_ERRORS_MAP(code) {#code, diagnostic::Code::code},
      FLUIR_DIAGNOSTIC_CODE(FLUIR_TEST_MAKE_ERRORS_MAP)
#undef FLUIR_TEST_MAKE_ERRORS_MAP
    };

    std::unordered_multimap<diagnostic::Code, int> errors;
    std::ifstream fin(file);

    for (std::string line; std::getline(fin, line);) {
      auto codeName = line.substr(0, line.find(' '));
      auto code = diagnostic::Code::GENERIC_ERROR;
      if (translation.contains(codeName)) {
        code = translation.at(codeName);
      }
      int lineNo{};
      auto i = codeName.size() + 1;
      if (i >= line.size()) {
        errors.emplace(code, std::move(lineNo));
        continue;
      }
      std::string lineStr = line.substr(i, line.find(' ', i));
      std::from_chars(lineStr.data(), lineStr.data() + lineStr.size(), lineNo);
      errors.emplace(code, lineNo);
    }
    return errors;
  }

  inline std::vector<std::filesystem::path> getTestPrograms(const std::filesystem::path& relative) {
    std::filesystem::path absoluteParent = TEST_FOLDER / "programs" / relative;
    std::vector<std::filesystem::path> programs;

    for (const auto& entry : std::filesystem::directory_iterator(absoluteParent)) {
      if (std::filesystem::is_regular_file(entry) && std::filesystem::path(entry).extension() == ".fl") {
        programs.emplace_back(entry.path());
      }
    }

    return programs;
  }

  inline std::string filePathName(const ::testing::TestParamInfo<std::filesystem::path>& info) {
    return info.param.stem().string();
  }
}  // namespace fluir::test

#endif
