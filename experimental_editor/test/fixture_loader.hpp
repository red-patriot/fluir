#pragma once

#include <filesystem>
#include <string>

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
   *  declare <version>0.1.3</version>). */
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

}  // namespace testutil
