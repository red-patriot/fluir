#ifndef FLUIR_COMPILER_UTILITY_DIAGNOSTIC_PRETTY_MSG_HPP
#define FLUIR_COMPILER_UTILITY_DIAGNOSTIC_PRETTY_MSG_HPP

#include <string>

#include "compiler/utility/diagnostic/code.hpp"

namespace fluir::diagnostic {
  std::string prettyMessage(Code code);

}

#endif
