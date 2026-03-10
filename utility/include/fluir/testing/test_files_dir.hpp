#ifndef FLUIR_UTILITY_TESTING_TEST_FILES_HPP
#define FLUIR_UTILITY_TESTING_TEST_FILES_HPP

#include <filesystem>
#include <vector>

namespace fluir::test {
  /** Gets all test programs in the given directory relative to fluir/test_programs */
  std::vector<std::filesystem::path> getTestPrograms(const std::filesystem::path& relative);
  std::filesystem::path getRelativePath(const std::filesystem::path& programFile);
}  // namespace fluir::test

#endif
