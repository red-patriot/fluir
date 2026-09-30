#ifndef FLUIR_EDITOR_TRANSACTION_ADD_CONDUIT_HPP
#define FLUIR_EDITOR_TRANSACTION_ADD_CONDUIT_HPP

#include <memory>
#include <optional>
#include <utility>

#include "compiler/models/id.hpp"
#include "editor/core/tree.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Adds one-branch conduit `newId` from `source`'s output to `target`'s input in `parent`'s body.
   *  A target already fed loses that branch first. */
  class AddConduit : public Transaction {
   public:
    /** A node or rail id and its terminal index. */
    struct Endpoint {
      fluir::ID node;
      int index;
    };

    AddConduit(fluir::FullID parent, fluir::ID newId, Endpoint source, Endpoint target) :
      parent_(std::move(parent)), id_(newId), source_(source), target_(target) { }

    bool execute(et::ParseTree& tree) override;
    bool unexecute(et::ParseTree& tree) override;

   private:
    fluir::FullID parent_;
    fluir::ID id_;
    Endpoint source_;
    Endpoint target_;
    std::optional<et::Conduit> replaced_; /**< the target's old conduit, as it was */
  };

  std::unique_ptr<Transaction> addConduit(fluir::FullID parent,
                                          fluir::ID newId,
                                          AddConduit::Endpoint source,
                                          AddConduit::Endpoint target);

}  // namespace fluir::editor

#endif
