#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/actors/node_actor.hpp"
#include "editor/components/text_field.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Actor for a `pt::Binary` node: clicking summarizes the stringified operator. */
  class BinaryActor : public NodeActor {
   public:
    BinaryActor(fluir::ID functionId, pt::Binary node, Rect bound);

    void onClick(Vec2 position) override;
    PortSet ports(const EditorContext& ctx) const override;
    using NodeActor::location;
    fluir::FlowGraphLocation* location() override { return &node_.location; }
    pt::Node node() const override { return node_; }

    std::vector<int> clearOperands(fluir::ID nodeId) override;
    void restoreOperand(int slot, fluir::ID nodeId) override;

    const std::string& lastClickSummary() const { return lastClickSummary_; }

   protected:
    void drawSelf(const Subview& body, const EditorContext& ctx) const override;

   private:
    pt::Binary node_;
    std::string lastClickSummary_;
  };

  /** Actor for a `pt::Unary` node: clicking summarizes the stringified operator. */
  class UnaryActor : public NodeActor {
   public:
    UnaryActor(fluir::ID functionId, pt::Unary node, Rect bounds);

    void onClick(Vec2 position) override;
    PortSet ports(const EditorContext& ctx) const override;
    using NodeActor::location;
    fluir::FlowGraphLocation* location() override { return &node_.location; }
    pt::Node node() const override { return node_; }

    std::vector<int> clearOperands(fluir::ID nodeId) override;
    void restoreOperand(int slot, fluir::ID nodeId) override;

    const std::string& lastClickSummary() const { return lastClickSummary_; }

   protected:
    void drawSelf(const Subview& body, const EditorContext& ctx) const override;

   private:
    pt::Unary node_;
    std::string lastClickSummary_;
  };

  /** Actor for a `pt::Constant` node: clicking summarizes the literal value. */
  class ConstantActor : public NodeActor {
   public:
    ConstantActor(fluir::ID functionId, pt::Constant node, Rect bounds);

    void onClick(Vec2 position) override;
    PortSet ports(const EditorContext& ctx) const override;
    using NodeActor::location;
    fluir::FlowGraphLocation* location() override { return &node_.location; }
    pt::Literal* literal() { return &node_.value; }
    pt::Node node() const override { return node_; }

    bool onFocus(const EditorContext& ctx, Vec2 position) override;
    void onBlur() override;
    bool onKey(const EditorContext& ctx, InputEvent::Key key) override;
    bool onTextInput(const EditorContext& ctx, std::string_view text) override;

    /** This constant's in-place editor, for observing an open draft. */
    const TextField& field() const { return field_; }

    /** This constant's value as editable text, or nullopt when its type is not editable. */
    std::optional<std::string> editableText() const;

    /** Parses `text` and raises the edit. False rejects the draft. */
    bool applyText(const EditorContext& ctx, const std::string& text);

    const std::string& lastClickSummary() const { return lastClickSummary_; }

   protected:
    void drawSelf(const Subview& body, const EditorContext& ctx) const override;

   private:
    pt::Constant node_;
    std::string lastClickSummary_;
    TextField field_;
  };

  /** Actor for a `pt::Call` node: clicking summarizes the call's target name. */
  class CallActor : public NodeActor {
   public:
    CallActor(fluir::ID functionId, pt::Call node, Rect bounds);

    void onClick(Vec2 position) override;
    PortSet ports(const EditorContext& ctx) const override;
    using NodeActor::location;
    fluir::FlowGraphLocation* location() override { return &node_.location; }
    pt::Node node() const override { return node_; }

    const std::string& lastClickSummary() const { return lastClickSummary_; }

   protected:
    void drawSelf(const Subview& body, const EditorContext& ctx) const override;

   private:
    pt::Call node_;
    std::string lastClickSummary_;
  };

}  // namespace fluir::editor
