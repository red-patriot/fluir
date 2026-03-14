#include "fluir/testing/test_files_dir.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
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

  std::string readContents(const std::filesystem::path& file) {
    std::fstream fin(file);
    std::stringstream ss;
    ss << fin.rdbuf();
    return ss.str();
  }

  std::string filePathName(const ::testing::TestParamInfo<std::filesystem::path>& info) {
    return info.param.stem().string();
  }
}  // namespace fluir::test
