#ifndef FLUIR_EDITOR_TRANSACTION_ADD_CONDUIT_HPP
#define FLUIR_EDITOR_TRANSACTION_ADD_CONDUIT_HPP

#include <optional>
#include <utility>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Adds one-branch conduit `newId` from `source`'s output to `target`'s input in `parent`'s body.
   *  A target already fed loses that branch first. */
  class AddConduit : public Transaction {
   public:
    /** A node or rail id and its port index. */
    struct Endpoint {
      fluir::ID node;
      int index;
    };

    AddConduit(fluir::FullID parent, fluir::ID newId, Endpoint source, Endpoint target) :
      parent_(std::move(parent)), id_(newId), source_(source), target_(target) { }

    bool execute(pt::ParseTree& tree) override;
    bool unexecute(pt::ParseTree& tree) override;

   private:
    fluir::FullID parent_;
    fluir::ID id_;
    Endpoint source_;
    Endpoint target_;
    std::optional<pt::Conduit> replaced_; /**< the target's old conduit, as it was */
  };

}  // namespace fluir::editor

#endif
