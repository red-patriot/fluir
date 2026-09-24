#ifndef FLUIR_EDITOR_CORE_FIELDS_HPP
#define FLUIR_EDITOR_CORE_FIELDS_HPP

#include <memory>
#include <optional>
#include <string>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/core/field.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor::fields {

  /** `field`'s text on the node or declaration at `path`; nullopt when it has no such text field. */
  std::optional<std::string> read(const pt::ParseTree& tree, const FullID& path, Field field);

  /** The edit that sets `field` to `text`: nullopt rejects `text`, a null edit means the value is unchanged. */
  std::optional<std::unique_ptr<Transaction>> write(const pt::ParseTree& tree,
                                                    const FullID& path,
                                                    Field field,
                                                    const std::string& text);

}  // namespace fluir::editor::fields

#endif
