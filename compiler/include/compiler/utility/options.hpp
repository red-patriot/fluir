#ifndef FLUIR_COMPILER_UTILITY_OPTIONS_HPP
#define FLUIR_COMPILER_UTILITY_OPTIONS_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace fluir {
  struct CompilerOptions {
    std::filesystem::path inputFilename;
    std::filesystem::path outputFilename{"out.flc"};
    bool colorOutput{true};

    friend bool operator==(const CompilerOptions&, const CompilerOptions&) = default;
  };

  CompilerOptions parseArgs(int argc, const char** argv);
  CompilerOptions parseArgs(const std::vector<std::string>& args);

}  // namespace fluir

#endif
