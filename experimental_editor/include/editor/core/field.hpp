#ifndef FLUIR_EDITOR_CORE_FIELD_HPP
#define FLUIR_EDITOR_CORE_FIELD_HPP

namespace fluir::editor {

  /** One editable value of a node or declaration; `index` names the argument or parameter it belongs to. */
  struct Field {
    enum class Kind { Name, ParamName, Target, Arg, Literal, Bool, Text, Operator };

    Kind kind;
    int index = 0;

    friend bool operator==(const Field&, const Field&) = default;
  };

}  // namespace fluir::editor

#endif
