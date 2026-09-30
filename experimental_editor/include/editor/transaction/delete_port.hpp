#ifndef FLUIR_EDITOR_TRANSACTION_DELETE_PORT_HPP
#define FLUIR_EDITOR_TRANSACTION_DELETE_PORT_HPP

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "compiler/models/id.hpp"
#include "editor/core/tree.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Deletes a conditional's non-condition port with its conduits. */
  class DeletePort : public Transaction {
   public:
    DeletePort(fluir::FullID path, PortRef ref) : path_(std::move(path)), ref_(ref) { }

    bool execute(et::ParseTree& tree) override;
    bool unexecute(et::ParseTree& tree) override;

   private:
    fluir::FullID path_;
    PortRef ref_;
    std::optional<et::BlockPort> port_;
    std::vector<et::Conduit> outer_;                   /**< parent block conduits touching the conditional */
    std::array<std::vector<et::Conduit>, 2> branches_; /**< then, else: conduits touching the inner id */
  };

  std::unique_ptr<Transaction> deletePort(fluir::FullID path, PortRef ref);

}  // namespace fluir::editor

#endif
