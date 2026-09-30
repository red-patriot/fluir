#ifndef FLUIR_EDITOR_TRANSACTION_ADD_PORT_HPP
#define FLUIR_EDITOR_TRANSACTION_ADD_PORT_HPP

#include <memory>
#include <utility>

#include "compiler/models/id.hpp"
#include "editor/core/tree.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Appends a port to conditional's inputs or outputs. */
  class AddPort : public Transaction {
   public:
    AddPort(fluir::FullID path, bool output, fluir::ID innerId, int y) :
      path_(std::move(path)), output_(output), innerId_(innerId), y_(y) { }

    bool execute(et::ParseTree& tree) override;
    bool unexecute(et::ParseTree& tree) override;

   private:
    fluir::FullID path_;
    bool output_;
    fluir::ID innerId_;
    int y_;
  };

  std::unique_ptr<Transaction> addPort(fluir::FullID path, bool output, fluir::ID innerId, int y);

}  // namespace fluir::editor

#endif
