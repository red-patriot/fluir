#ifndef FLUIR_UTILITY_TESTING_TEST_FILES_HPP
#define FLUIR_UTILITY_TESTING_TEST_FILES_HPP

#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

namespace fluir::test {
  /** Gets all test programs in the given directory relative to fluir/test_programs */
  std::vector<std::filesystem::path> getTestPrograms(const std::filesystem::path& relative);
  /** Gets the relative path of a file inside the test program directory */
  std::filesystem::path getRelativePath(const std::filesystem::path& programFile);
  /** Reads the contents of a file into a string */
  std::string readContents(const std::filesystem::path& file);
  /** Gets the name of a test file */
  std::string filePathName(const ::testing::TestParamInfo<std::filesystem::path>& info);
}  // namespace fluir::test

#endif
