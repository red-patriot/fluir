#ifndef FLUIR_EDITOR_DIAGRAM_TERMINAL_HPP
#define FLUIR_EDITOR_DIAGRAM_TERMINAL_HPP

#include <QGraphicsItem>
#include <QRectF>
#include <vector>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::diagram {

  /** Where a conduit attaches. */
  class Terminal : public QGraphicsItem {
   public:
    /** A terminal centered on `anchor`, in `owner`'s coordinates. */
    Terminal(bool output, int index, Vec2 anchor, const EditorContext& ctx, QGraphicsItem* owner);

    bool isOutput() const { return output_; }
    /** Its index on its side of its endpoint. */
    int index() const { return index_; }

    QRectF boundingRect() const override { return rect_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    const EditorContext& ctx_;
    QRectF rect_;
    bool output_;
    int index_;
  };

  /** One endpoint's terminals in index order. */
  struct Terminals {
    std::vector<Terminal*> inputs;
    std::vector<Terminal*> outputs;
  };

  /** Adds a terminal to `owner` at each of `anchors` (in `owner`'s coordinates). */
  Terminals addTerminals(QGraphicsItem& owner, const TerminalSet& anchors, const EditorContext& ctx);

}  // namespace fluir::editor::diagram

#endif
