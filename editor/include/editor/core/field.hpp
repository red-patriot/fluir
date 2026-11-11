#ifndef FLUIR_EDITOR_CORE_FIELD_HPP
#define FLUIR_EDITOR_CORE_FIELD_HPP

namespace fluir::editor {

  /** One editable value of a node or declaration; `index` names the argument or parameter it belongs to. */
  struct Field {
    enum class Kind { Name, ParamName, ParamType, ReturnType, Target, Arg, Literal, Bool, Text, Operator };

    Kind kind;
    int index = 0;

    friend bool operator==(const Field&, const Field&) = default;
  };

  /** Whether `kind` is picked from a menu rather than typed. */
  constexpr bool isChoice(Field::Kind kind) {
    return kind == Field::Kind::Operator || kind == Field::Kind::ParamType || kind == Field::Kind::ReturnType;
  }

}  // namespace fluir::editor

#endif
