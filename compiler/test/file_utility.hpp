#ifndef FLUIR_COMPILER_TEST_FILE_UTILITY_HPP
#define FLUIR_COMPILER_TEST_FILE_UTILITY_HPP

#include <filesystem>
#include <fstream>
#include <vector>

#include <gtest/gtest.h>

namespace fluir::test {
  inline std::string readContents(const std::filesystem::path& file) {
    std::fstream fin(file);
    std::stringstream ss;
    ss << fin.rdbuf();
    return ss.str();
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
