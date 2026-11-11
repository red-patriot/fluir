#ifndef FLUIR_EDITOR_TEST_FIXTURE_LOADER_HPP
#define FLUIR_EDITOR_TEST_FIXTURE_LOADER_HPP

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/utility/context.hpp"
#include "editor/core/collecting_sink.hpp"
#include "editor/core/loader.hpp"

namespace testutil {

  /** A loaded fixture plus the sink that collected its diagnostics; the sink
   *  must outlive the result, so they travel together. */
  struct Loaded {
    fluir::editor::CollectingSink sink;
    fluir::editor::LoadResult result;
  };

  /** Loads `relPath` under TEST_FOLDER with version checks off (fixtures
   *  declare <version>0.1.3</version>). An absolute path is loaded as is. */
  inline Loaded loadFixture(const std::string& relPath) {
    Loaded l;
    fluir::Context ctx{
      .diagnosticSink = l.sink,
      .symbolTable = {},
      .currentFile = {},
      .outputFilename = {},
      .version = {},
      .ignoreVersionChecks = true,
    };
    l.result = fluir::editor::loadFile(ctx, std::filesystem::path(TEST_FOLDER) / relPath);
    return l;
  }

  // Read/write fixtures the current compiler frontend cannot parse yet. When parser
  // support lands for one, remove it here so it moves into the loadable set.
  //
  //   branching_conduits, conduits       -- use <segment>, which the parser rejects
  //                                         with ERROR_UNEXPECTED_ELEMENT.
  //   function_with_input_and_output     -- declares two <return> elements in one
  //                                         <output>; the parser allows a single
  //                                         return only (ERROR_TOO_MANY_RETURNS).
  inline const std::vector<std::string> UNPARSEABLE{"branching_conduits", "conduits", "function_with_input_and_output"};

  /** The .fl files in `sub` under TEST_FOLDER, sorted. */
  inline std::vector<std::filesystem::path> fixturesIn(const std::string& sub) {
    std::vector<std::filesystem::path> fixtures;
    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::path(TEST_FOLDER) / sub)) {
      if (entry.is_regular_file() && entry.path().extension() == ".fl") {
        fixtures.push_back(entry.path());
      }
    }
    std::ranges::sort(fixtures);
    return fixtures;
  }

  /** Read and write fixtures, split by whether the parser accepts them. */
  inline std::vector<std::filesystem::path> readWriteFixtures(bool parseable) {
    std::vector<std::filesystem::path> out;
    for (const char* sub : {"read", "write"}) {
      for (const auto& path : fixturesIn(sub)) {
        if (std::ranges::contains(UNPARSEABLE, path.stem().string()) != parseable) {
          out.push_back(path);
        }
      }
    }
    return out;
  }

  /** "<dir>_<stem>", a test name unique across fixture directories. */
  inline std::string fixtureName(const testing::TestParamInfo<std::filesystem::path>& info) {
    return info.param.parent_path().filename().string() + "_" + info.param.stem().string();
  }

}  // namespace testutil

#endif
