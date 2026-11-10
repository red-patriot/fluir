#ifndef FLUIR_EDITOR_DIAGRAM_EDIT_TARGET_HPP
#define FLUIR_EDITOR_DIAGRAM_EDIT_TARGET_HPP

#include <string>
#include <string_view>
#include <vector>

#include "editor/diagram/field_part.hpp"

namespace fluir::editor::diagram {

  class FluirNode;

  /** The target for what an edit changes. */
  class EditTarget {
   public:
    virtual ~EditTarget() = default;

    /** The current value as text. */
    virtual std::string current() const = 0;
    /** Check if the given text is valid for the target of an edit */
    virtual bool validate(std::string_view text) const = 0;
    /** The values a pick offers. */
    virtual std::vector<std::string> options() const = 0;
    /** Changes the value to `text` as one edit. */
    virtual void commit(std::string_view text) = 0;
  };

  /** One field of a node, edited through `SetField`. */
  class FieldTarget : public EditTarget {
   public:
    FieldTarget(FluirNode& node, FieldKey key);

    std::string current() const override;
    bool validate(std::string_view text) const override;
    std::vector<std::string> options() const override;
    void commit(std::string_view text) override;

   private:
    FluirNode& node_;
    FieldKey key_;
  };

}  // namespace fluir::editor::diagram

#endif
