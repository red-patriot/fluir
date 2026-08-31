#include "editor/core/loader.hpp"

#include <exception>
#include <utility>

#include "compiler/frontend/parser.hpp"
#include "compiler/utility/context.hpp"
#include "compiler/utility/results.hpp"
#include "editor/core/collecting_sink.hpp"

namespace fluir::editor {
  namespace {

    template <typename ParseFn>
    LoadResult runParse(fluir::Context& ctx, ParseFn&& parse) {
      Results<pt::ParseTree> result = parse(ctx);
      return LoadResult{std::move(result)};
    }

  }  // namespace

  LoadResult loadFile(fluir::Context& ctx, const std::filesystem::path& path) {
    return runParse(ctx, [&](Context& c) { return parseFile(c, path); });
  }

  LoadResult loadString(fluir::Context& ctx, std::string_view source) {
    return runParse(ctx, [&](Context& c) { return parseString(c, source); });
  }

}  // namespace fluir::editor
