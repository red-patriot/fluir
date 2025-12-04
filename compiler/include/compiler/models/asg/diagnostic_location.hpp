#ifndef FLUIR_COMPILER_MODELS_ASG_DIAGNOSTIC_LOCATION_HPP
#define FLUIR_COMPILER_MODELS_ASG_DIAGNOSTIC_LOCATION_HPP

#include <string>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "compiler/models/asg/node.hpp"
#include "compiler/utility/diagnostics.hpp"

namespace fluir::asg {
  class DiagnosticLocation : public ::fluir::Diagnostic::Location {
   public:
    DiagnosticLocation(std::string_view filename, Node const* node) : filename_(filename), id_(node->fullId()) { }

    std::string str() const override { return fmt::format("in element '{}' of '{}'", fmt::join(id_, ":"), filename_); }

   private:
    std::string filename_;
    FullID id_;
  };
}  // namespace fluir::asg

#endif
