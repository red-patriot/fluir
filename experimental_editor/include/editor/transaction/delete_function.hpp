#ifndef FLUIR_EDITOR_TRANSACTION_DELETE_FUNCTION_HPP
#define FLUIR_EDITOR_TRANSACTION_DELETE_FUNCTION_HPP

#include "compiler/models/id.hpp"
#include "editor/actors/scene.hpp"
#include "editor/transaction/transaction.hpp"

namespace fluir::editor {

  /** Deletes one function frame; its whole body goes with it. */
  class DeleteFunctionTransaction : public Transaction {
   public:
    explicit DeleteFunctionTransaction(fluir::ID functionId) : functionId_(functionId) { }

    bool execute(GraphScene& scene) override;
    bool unexecute(GraphScene& scene) override;

   private:
    fluir::ID functionId_;
    GraphScene::DetachedActor removed_;
  };

}  // namespace fluir::editor

#endif
