#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <ostream>

#include "compiler/frontend/parse_tree/parse_tree.hpp"

namespace fluir::editor {

  /** Serializes a parse tree to XML matching the editor's on-disk format. */
  class ParseTreeWriter {
   public:
    /** Writes to `out`, which the caller owns. */
    explicit ParseTreeWriter(std::ostream& out);

    void write(const fluir::pt::ParseTree& tree);

    /** State of the underlying stream (false => a write or open failed). */
    [[nodiscard]] bool good() const;

   private:
    std::ostream& out_; /**< Stream out for string result */
  };

}  // namespace fluir::editor
