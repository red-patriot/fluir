#ifndef FLUIR_COMPILER_UTILITY_OPTIONS_HPP
#define FLUIR_COMPILER_UTILITY_OPTIONS_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace fluir {
  struct CompilerOptions {
    std::filesystem::path inputFilename;             /**< The file to compile */
    std::filesystem::path outputFilename{"out.flc"}; /**< The output to write */
    bool colorOutput{true};                          /**< Whether to print diagnostics in color if supported */

    friend bool operator==(const CompilerOptions&, const CompilerOptions&) = default;
  };

  std::optional<CompilerOptions> parseArgs(int argc, const char** argv);
  std::optional<CompilerOptions> parseArgs(const std::vector<std::string>& args);

}  // namespace fluir

#endif
