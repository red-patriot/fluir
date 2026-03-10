#include "fluir/testing/test_files_dir.hpp"

#include <filesystem>
#include <vector>

namespace fluir::test {
  static const std::filesystem::path ABSOLUTE_PROGRAMS_DIR = std::filesystem::path{TEST_PROGRAMS_DIRECTORY};

  std::vector<std::filesystem::path> getTestPrograms(const std::filesystem::path& relative) {
    std::filesystem::path absoluteParent = ABSOLUTE_PROGRAMS_DIR / relative;
    std::vector<std::filesystem::path> programs;

    for (const auto& entry : std::filesystem::directory_iterator(absoluteParent)) {
      if (std::filesystem::is_regular_file(entry) && std::filesystem::path(entry).extension() == ".fl") {
        programs.emplace_back(entry.path());
      }
    }

    return programs;
  }

  std::filesystem::path getRelativePath(const std::filesystem::path& programFile) {
    return std::filesystem::relative(programFile, ABSOLUTE_PROGRAMS_DIR);
  }
}  // namespace fluir::test
