#ifndef FLUIR_UTILITY_DIAGNOSTIC_CODE_HPP
#define FLUIR_UTILITY_DIAGNOSTIC_CODE_HPP

#include <cstdint>

namespace fluir::diagnostic {
  // clang-format off
#define FLUIR_DIAGNOSTIC_CODE(x)                  \
    x(GENERIC_NOTE)                               \
    x(GENERIC_WARNING)                            \
    x(WARNING_UNKNOWN)                            \
    x(GENERIC_ERROR)                              \
    x(ERROR_UNKNOWN)                              \
    x(ERROR_WRONG_ROOT_ELEMENT)                   \
    x(ERROR_MISSING_MODULE_HEADER)                \
    x(ERROR_UNEXPECTED_DUPLICATE_HEADER)          \
    x(ERROR_INCORRECT_MODULE_VERSION)             \
    x(ERROR_FILE_DOES_NOT_EXIST)                  \
    x(ERROR_UNEXPECTED_ELEMENT)                   \
    x(ERROR_MISSING_ELEMENT)                      \
    x(ERROR_MISSING_ATTRIBUTE)                    \
    x(ERROR_CANNOT_PARSE_ELEMENT_TEXT)            \
    x(ERROR_UNRECOGNIZED_OPERATOR)                \
    x(ERROR_DUPLICATE_IDS_FOUND)                  \
    x(ERROR_NUMBER_OUT_OF_RANGE)                  \
    x(ERROR_CIRCULAR_DEPENDENCY)                  \
    x(ERROR_OPERATOR_OVERLOAD_RESOLUTION_FAILED)  \
    x(ERROR_CANNOT_DETERMINE_TYPE_OF_LOCAL)
  // clang-format on

  enum class Code : std::uint16_t {
#define FLUIR_ENUMERATE(x) x,
    FLUIR_DIAGNOSTIC_CODE(FLUIR_ENUMERATE)
#undef FLUIR_ENUMERATE
  };

  inline bool isWarning(Code code) { return code >= Code::GENERIC_WARNING && code < Code::GENERIC_ERROR; }
  inline bool isError(Code code) { return code >= Code::GENERIC_ERROR; }
}  // namespace fluir::diagnostic

#endif
