#include "compiler/utility/diagnostic/pretty_msg.hpp"

#include <unordered_map>
#include <utility>

#include <fmt/format.h>

namespace fluir::diagnostic {
  namespace {
    using enum Code;

    const std::unordered_map<Code, std::string> messages{
      {GENERIC_NOTE, ""},
      {GENERIC_WARNING, "An unknown warning was emitted."},
      {GENERIC_ERROR, "An unknown error was emitted."},
      {ERROR_WRONG_ROOT_ELEMENT, "The root of the XML tree must be <fluir>."},
      {ERROR_MISSING_MODULE_HEADER, "The module is missing a <header> element."},
      {ERROR_UNEXPECTED_DUPLICATE_HEADER,
       "Multiple <header> elements were found in the module. "
       "Exactly one is allowed."},
      {ERROR_INCORRECT_MODULE_VERSION, "Module version mismatch."},
      {ERROR_FILE_DOES_NOT_EXIST, "The file was not found on disk."},
      {ERROR_UNEXPECTED_ELEMENT, "Encountered an unexpected XML element."},
      {ERROR_MISSING_ELEMENT, "Missing a required XML element."},
      {ERROR_MISSING_ATTRIBUTE, "Element XML is missing a required attribute"},
      {ERROR_CANNOT_PARSE_ELEMENT_TEXT, "Cannot parse the text of the XML element into its expected representation."},
      {ERROR_UNRECOGNIZED_OPERATOR, "The given operator is not recognized as one of the allowed operators."},
      {ERROR_DUPLICATE_IDS_FOUND,
       "Multiple elements with the same ID were found. "
       "IDs must be unique."},
      {ERROR_NUMBER_OUT_OF_RANGE,
       "The number was parsed successfully, "
       "but does not fit into the range pf its target type."},
      {ERROR_CIRCULAR_DEPENDENCY, "There is a circular dependency between flow graph nodes."},
      {ERROR_OPERATOR_OVERLOAD_RESOLUTION_FAILED, "Operator overload resolution failed."}};

  }  // namespace

  std::string prettyMessage(Code code) {
    if (messages.contains(code)) {
      return messages.at(code);
    }

    // If there isn't a known message, just print out the number
    return fmt::format("ERROR 0x{:X}", std::to_underlying(code));
  }
}  // namespace fluir::diagnostic
