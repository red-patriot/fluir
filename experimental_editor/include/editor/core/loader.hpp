#pragma once

#include <filesystem>
#include <optional>
#include <string_view>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/utility/context.hpp"

namespace fluir::editor {

  /** The outcome of loading a fluir file. */
  struct LoadResult {
    std::optional<pt::ParseTree> tree; /**< std::nullopt => parse failed */
  };

  /** Parses a fluir file at `path`.
   *
   * The caller owns and configures `ctx`: bind `ctx.diagnosticSink` to a sink (a
   * CollectingSink to capture the messages) and set `ctx.ignoreVersionChecks = true`
   * (fixtures declare <version>0.1.3</version>; without it the parse fails with
   * ERROR_INCORRECT_MODULE_VERSION). This function does not touch any `ctx` fields.
   */
  LoadResult loadFile(fluir::Context& ctx, const std::filesystem::path& path);

  /** Parses fluir source text.
   *
   * The caller owns and configures `ctx` exactly as for loadFile (see above); this
   * function does not touch any `ctx` fields.
   */
  LoadResult loadString(fluir::Context& ctx, std::string_view source);

}  // namespace fluir::editor
