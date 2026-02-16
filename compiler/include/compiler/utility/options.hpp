#ifndef FLUIR_COMPILER_UTILITY_OPTIONS_HPP
#define FLUIR_COMPILER_UTILITY_OPTIONS_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace fluir {
  struct DeveloperOptions {
    bool printAST{false};
    bool suppressVersionErrors{false};

    friend bool operator==(const DeveloperOptions&, const DeveloperOptions&) = default;
  };

  struct CompilerOptions {
    std::filesystem::path inputFilename;             /**< The file to compile */
    std::filesystem::path outputFilename{"out.flc"}; /**< The output to write */
    bool colorOutput{true};                          /**< Whether to print diagnostics in color if supported */
    DeveloperOptions developerOptions{};             /**< Additional (hidden) options for development */

    friend bool operator==(const CompilerOptions&, const CompilerOptions&) = default;
  };

  std::optional<CompilerOptions> parseArgs(int argc, const char** argv);
  std::optional<CompilerOptions> parseArgs(const std::vector<std::string>& args);

}  // namespace fluir

#endif
