#ifndef FLUIR_EDITOR_TRANSACTION_TRANSACTION_HPP
#define FLUIR_EDITOR_TRANSACTION_TRANSACTION_HPP

namespace fluir::editor {
  class GraphScene;

  /** A reversible edit to a scene. Redo re-calls `execute`, so `execute` must be
   *  valid from the state `unexecute` leaves behind. */
  class Transaction {
   public:
    virtual ~Transaction() = default;

    /** Applies the edit. False when nothing changed. */
    virtual bool execute(GraphScene& scene) = 0;
    /** Reverses `execute`. False when the scene is not in the expected state. */
    virtual bool unexecute(GraphScene& scene) = 0;
  };
}  // namespace fluir::editor

#endif
